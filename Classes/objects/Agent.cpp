//
//  Agent.cpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "app_constants.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "FirePattern.hpp"
#include "FirePatternImpl.hpp"
#include "FloorSegment.hpp"
#include "GAnimation.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GraphicsNodes.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "MiscMagicEffects.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"
#include "value_map.hpp"

const Color4F Agent::bodyOutlineColor = hsva4F(270.0f, 0.2f, 0.7f, 0.667f);
const Color4F Agent::shieldConeColor = Color4F(.37f, .56f, .57f, 0.5f);
const float Agent::bodyOutlineWidth = 4.0f;

agent_attributes Agent::parseAttributes(const ValueMap& args)
{
	agent_attributes result;

	result.ai_package = getStringOrDefault(args, "ai_package", "");
	result.level = getIntOrDefault(args, "level", 0);
	result.name = getStringOrDefault(args, "name", "");
	result.pos = getObjectPos(args);

	Direction dir = getDirectionOrDefault(args, Direction::none);
	if (dir != Direction::none) {
		result.angle = dirToPhysicsAngle(dir);
	}

	return result;
}

bool Agent::conditionalLoad(GSpace* space, const agent_attributes& attrs, local_shared_ptr<agent_properties> props)
{
	if (attrs.name.size() > 0 && App::crntState->isObjectRemoved(space->getCrntChamber(), attrs.name)) {
		return false;
	}

	auto& cls = space->scriptVM->_state["objects"][props->clsName];

	if (cls) {
		sol::function f = cls["conditionalLoad"];

		if (f && !f(space, attrs, props)) {
			log("object load canceled");
			return false;
		}
	}

	return true;
}

Agent::Agent(
	GSpace* space,
	ObjectIDType id,
	GType type,
	const agent_attributes& attr,
	local_shared_ptr<agent_properties> props
) :
	GObject(
		space,
		id,
		object_params(attr.pos, attr.angle),
		physics_params(
			type,
			props->isFlying ? PhysicsLayers::flying : PhysicsLayers::onGround,
			props->dimensions,
			props->mass
		)
	),
	props(props),
	level(attr.level)
{
	if (props->ai_package.empty() && attr.ai_package.empty() && type != GType::player) {
		log("%s: no AI package!", toString());
	}
	else {
		ai_package = attr.ai_package.size() > 0 ? attr.ai_package : props->ai_package;
	}
}

Agent::~Agent()
{
}

bullet_attributes Agent::getBulletAttributes(local_shared_ptr<bullet_properties> props) const
{
	bullet_attributes result;

	result.caster = gobject_ref(this);
	result.startRoom = crntRoom;
	result.casterVelocity = getVel();
	result.type = getType() == GType::player ? GType::playerBullet : GType::enemyBullet;
	result.attackDamage = getAttribute(Attribute::attack);
	result.bulletSpeed = getAttribute(Attribute::bulletSpeed);

	return result;
}

string Agent::getSprite() const
{
	return props->sprite;
}

boost::shared_ptr<LightArea> Agent::getLightSource() const
{
	return props->light;
}

bool Agent::hasEssenceRadar() const {
	return props->detectEssence;
}

SpaceFloat Agent::getRadarRadius() const {
	return props->viewRange;
}

SpaceFloat Agent::getDefaultFovAngle() const {
	return props->viewAngle;
}

void Agent::checkInitScriptObject()
{
	auto &cls = space->scriptVM->_state["objects"][getClsName()];

	if (cls) {
		scriptObj = cls(this);
	}
}

void Agent::initFSM()
{
	fsm = make_unique<ai::StateMachine>(this);

	auto it = ai::StateMachine::packages.find(ai_package);
	if (it != ai::StateMachine::packages.end()) {
		ValueMap args;

		auto f = it->second;
		f(fsm.get(), args);
	}
	else if(ai_package.size() > 0) {
		fsm->runScriptPackage(ai_package);
	}
	else{
		log("Agent %s, unknown AI package %s!", getName(), ai_package);
	}
}

void Agent::initAttributes()
{
	attributeSystem = getBaseAttributes();
	applyAttributeEffects(getAttributeUpgrades());

	space->graphicsNodeAction(
		&AgentBodyShader::setShieldLevel,
		agentOverlay,
		getAttribute(Attribute::shieldLevel)
	);
}

void Agent::init()
{
	GObject::init();

	initializeRadar();
	initFSM();
	initAttributes();
}

void Agent::update()
{
	GObject::update();

	if (attributeSystem[Attribute::hp] <= 0.0f && attributeSystem[Attribute::maxHP] >  0.0f) {
		onZeroHP();
	}
	if (attributeSystem[Attribute::stamina] <= 0.0f && attributeSystem[Attribute::maxStamina] > 0.0f) {
		fsm->onZeroStamina();
	}

	bool hasFreezeEffect = space->magicEffectSystem->hasScriptedEffect(this, "FreezeStatus");

	if (attributeSystem[Attribute::iceDamage] >= AttributeSystem::maxElementDamage && !hasFreezeEffect) {
		applyMagicEffect( app::getEffect("FreezeStatus"), 0.0f, 5.0f);
		attributeSystem.modifyAttribute(Attribute::iceDamage, -AttributeSystem::maxElementDamage);
	}
	if (attributeSystem[Attribute::sunDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}
	if (attributeSystem[Attribute::darknessDamage] >= AttributeSystem::maxElementDamage) {
		applyMagicEffect(app::getEffect("DarknessCurse"), 0.0f, -1.0f);
	}
	if (attributeSystem[Attribute::poisonDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}

	for (auto other : touchTargets) {
		other->hit(touchEffect(), ai::directionToTarget(this, other->getPos()));
	}

	if (firePattern) firePattern->update();
	attributeSystem.update(this);
	updateAgentOverlay();
	updateAnimation();
}

void Agent::sendAlert(Player* p)
{
	if(fsm)
		fsm->onAlert(p);
}

void Agent::onDetect(GObject* obj)
{
	if (obj->getType() == GType::playerBullet) {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromDetects);
	}

	if(fsm)
		fsm->onDetect(obj);
}

void Agent::onEndDetect(GObject* obj)
{
	if(fsm)
		fsm->onEndDetect(obj);
}

void Agent::onZeroHP()
{
	fsm->onZeroHP();
	space->removeObject(this);
}

void Agent::onRemove()
{
	GObject::onRemove();

	space->addGraphicsAction(&graphics_context::removeSprite, agentOverlay);
}

bool Agent::applyInitialSpellCost(const spell_cost& cost)
{
	float mpCost = cost.initial_mp;
	float staminaCost = cost.initial_stamina;

	if (mpCost > attributeSystem[Attribute::mp] || staminaCost > attributeSystem[Attribute::stamina])
		return false;

	attributeSystem.modifyAttribute(Attribute::mp, -mpCost);
	attributeSystem.modifyAttribute(Attribute::stamina, -staminaCost);

	return true;
}

bool Agent::applyOngoingSpellCost(const spell_cost& cost)
{
	float mpCost = cost.ongoing_mp * app::params.secondsPerFrame;
	float staminaCost = cost.ongoing_stamina * app::params.secondsPerFrame;

	if (mpCost > attributeSystem[Attribute::mp] || staminaCost > attributeSystem[Attribute::stamina]) {
		return false;
	}

	attributeSystem.modifyAttribute(Attribute::stamina, -staminaCost);
	attributeSystem.modifyAttribute(Attribute::mp, -mpCost);

	return true;
}

AttributeMap Agent::getBaseAttributes() const
{
	return !props->attributes.empty() ? app::getAttributes(props->attributes) : AttributeMap();
}

float Agent::getAttribute(Attribute id) const
{
	return attributeSystem[id];
}

void Agent::modifyAttribute(Attribute id, float val)
{
	attributeSystem.modifyAttribute(id, val);
}

bool Agent::setFirePattern(string firePattern)
{
	auto props = app::getFirePattern(firePattern);

	if (props) {
		this->firePattern = make_local_shared<FirePatternImpl>(this, props);
		return true;
	}
	else{
		auto it = FirePattern::playerFirePatterns.find(firePattern);
		if (it != FirePattern::playerFirePatterns.end()) {
			this->firePattern = it->second(this);
			return true;
		}
		else {
			log("Unknown fire pattern: %s", firePattern);
		}
	}

	return false;
}

SpaceFloat Agent::getTraction() const
{
	if (getAttribute(Attribute::iceSensitivity) >= 1.0f) {
		return GObject::getTraction();
	}
	else
	{
		SpaceFloat traction = GObject::getTraction();

		traction += (1.0 - traction) * (1.0 - getAttribute(Attribute::iceSensitivity));

		return traction;
	}
}

SpaceFloat Agent::getMaxSpeed() const
{
	return attributeSystem[Attribute::maxSpeed];
}

SpaceFloat Agent::getMaxAcceleration() const
{
	return attributeSystem[Attribute::maxAcceleration];
}

float Agent::getMaxHealth() const
{
	return attributeSystem[Attribute::maxHP];
}

int Agent::getHealth()
{
	return attributeSystem[Attribute::hp];
}

SpaceFloat Agent::getHealthRatio()
{
	if (attributeSystem[Attribute::maxHP] == 0.0f) {
		return 0.0;
	}

	return attributeSystem[Attribute::hp] / attributeSystem[Attribute::maxHP];
}

int Agent::getStamina()
{
	return attributeSystem[Attribute::stamina];
}

int Agent::getMagic()
{
	return attributeSystem[Attribute::mp];
}

bool Agent::consumeStamina(int val)
{
	if (getStamina() >= val) {
		attributeSystem.modifyAttribute(Attribute::stamina, -val);
		return true;
	}
	return false;
}

void Agent::setProtection()
{
	attributeSystem.setProtection();
}

void Agent::setTimedProtection(SpaceFloat seconds)
{
	attributeSystem.setTimedProtection(seconds);
}

void Agent::resetProtection()
{
	attributeSystem.resetProtection();
}

void Agent::setShieldActive(bool v)
{
	shieldActive = v;
}

bool Agent::isShield(Bullet * b)
{
	if (getAttribute(Attribute::shieldLevel) <= 0.0f || !shieldActive)
		return false;

	SpaceVect d = -1.0 * b->getVel().normalizeSafe();
	float cost = getShieldCost(d);

	if (cost == -1.0f || cost > getAttribute(Attribute::stamina))
		return false;

	modifyAttribute(Attribute::stamina, -cost);
	return true;
}

void Agent::initializeGraphics()
{
	animation = make_unique<AgentAnimationContext>(space);
	spriteID = animation->initializeGraphics(
		getSprite(),
		getRadius(),
		sceneLayer(),
		getInitialCenterPix()
	);
	animation->setAngle(prevAngle);

	agentOverlay = space->createSprite(
		&graphics_context::createAgentBodyShader,
		GraphicsLayer::agentOverlay,
		bodyOutlineColor,
		shieldConeColor,
		to_float(getRadius()*app::pixelsPerTile),
		to_float(Player::grazeRadius*app::pixelsPerTile),
		bodyOutlineWidth,
		getInitialCenterPix()
	);
	//Should be false, but in case shield has already been activated.
	space->graphicsNodeAction(&Node::setVisible, agentOverlay, false);
}

void Agent::setAngle(SpaceFloat a)
{
	GObject::setAngle(a);

	if (animation) {
		animation->setDirection(angleToDirection(a));
	}
}

void Agent::setDirection(Direction d)
{
	GObject::setDirection(d);

	if (animation) {
		animation->setDirection(d);
	}
}

void Agent::resetAnimation()
{
	if (animation) {
		animation->reset();
	}
}

void Agent::setSprite(const string& sprite)
{
	if (animation) {
		animation->setSprite(sprite);
	}
}

//shield
//1 - deflect 45deg for 10
//2 - deflect 90deg for 10 & 45deg for 5
//3 - deflect 135deg for 12.5, 90 for 7.5 & 45 deg for 2.5
//4 - deflect 180deg for 15, 135 for 10, 90 for 5 & 45 for 0

const vector<vector<float>> shieldCosts = {
	{10.0f, -1.0f, -1.0f, -1.0f},
	{5.0f, 10.0f, -1.0f, -1.0f},
	{2.5f, 7.5f, 12.5f, -1.0f},
	{0.0f, 5.0f, 10.0f, 15.0f},
};

const array<SpaceFloat, 4> shieldScalars = {
	boost::math::double_constants::one_div_root_two,
	0.0, 
	-boost::math::double_constants::one_div_root_two,
	-1.0
};

float Agent::getShieldCost(SpaceVect n)
{
	SpaceFloat scalar = SpaceVect::dot(n, SpaceVect::ray(1.0, getAngle()));
	int level = min<int>(attributeSystem[Attribute::shieldLevel], shieldCosts.size()+1);

	for_irange(i,0,shieldScalars.size()) {
		if (scalar >= shieldScalars[i]) {
			return shieldCosts[level - 1][i];
		}
	}

	//Shouldn't happen, since scalar product should always be >= -1.0
	return shieldCosts[3][level - 1];
}

void Agent::onBulletCollide(Bullet* b, SpaceVect n)
{
	DamageInfo damage = b->getScaledDamageInfo();

	if (!isShield(b)) {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromHits, damage.mag);
		hit(damage, n);
		if(fsm) fsm->onBulletHit(b);
	}
	else {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromBlocks, damage.mag);
		if(fsm) fsm->onBulletBlock(b);
	}
}

void Agent::onTouchAgent(Agent* other)
{
	if(getType() != other->getType())
		touchTargets.insert(other);
}

void Agent::onEndTouchAgent(Agent* other)
{
	touchTargets.erase(other);
}

bool Agent::hit(DamageInfo damage, SpaceVect n)
{
	if (attributeSystem.isNonzero(Attribute::hitProtection) || damage.mag == 0.0f)
		return false;

	if (dynamic_cast<Enemy*>(this)) {
		damage.mag /= app::params.difficultyScale;
	}
	else if (dynamic_cast<Player*>(this)) {
		damage.mag *= app::params.difficultyScale;
	}

	float hp = attributeSystem.applyDamage(damage);
	space->addGraphicsAction(&graphics_context::createDamageIndicator, hp, getPos());

	SpaceVect knockback = n * damage.knockback;

	if (!knockback.isZero() ) {
		applyImpulse(knockback);
	}

	return true;
}

void Agent::applyAttributeEffects(AttributeMap attributeEffect)
{
	attributeSystem.apply(attributeEffect);
}

DamageInfo Agent::touchEffect() const
{
	return DamageInfo();
}

void Agent::initializeRadar()
{
	auto attr = sensor_attributes{
		getRadarRadius(),
		getDefaultFovAngle(),
		GType::enemySensor,
		hasEssenceRadar()
	};

	if (attr.radius <= 0.0) {
		log("%s has zero radius", getName());
		return;
	}

	radar = new RadarSensor(
		this,
		attr,
		bind(&Agent::onDetect, this, placeholders::_1),
		bind(&Agent::onEndDetect, this, placeholders::_1)
	);

	if (radar) {
		space->insertSensor(radar);
	}
}

void Agent::removePhysicsObjects()
{
	GObject::removePhysicsObjects();

	if (radar) {
		space->removeSensor(radar);
		delete radar;
	}
}

void Agent::updateAgentOverlay()
{
	//	space->graphicsNodeAction(&Node::setVisible, agentOverlay, v);
	if ( (shieldActive && !wasShieldActive) || (!shieldActive && wasShieldActive) ) {
		space->graphicsNodeAction(&Node::setVisible, agentOverlay, shieldActive);
	}

	if (shieldActive) {
		space->addGraphicsAction(
			&graphics_context::setSpritePosition,
			agentOverlay,
			toCocos(getPos()*app::pixelsPerTile)
		);
		space->graphicsNodeAction(
			&Node::setRotation,
			agentOverlay,
			toCocosAngle(getAngle())
		);
	}
	wasShieldActive = shieldActive;
}

void Agent::updateAnimation()
{
	if (!animation) return;

	SpaceVect dist = getVel()*app::params.secondsPerFrame;
	bool advance = animation->accumulate(dist.length());

	if (advance && isOnFloor() && crntFloorCenterContact) {
		string sfxRes = crntFloorCenterContact->getFootstepSfx();
		if (!sfxRes.empty()) {
			playSoundSpatial(sfxRes, 0.5f, false, -1.0f);
		}
	}
}
