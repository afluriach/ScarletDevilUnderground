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
#include "Bomb.hpp"
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

bool Agent::conditionalLoad(GSpace* space, const object_params& params, local_shared_ptr<agent_properties> props)
{
	if (params.name.size() > 0 && App::crntState->isObjectRemoved(space->getCrntChamber(), params.name)) {
		return false;
	}

    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[props->clsName];

    if (cls.valid()) {
		sol::function f = cls["conditionalLoad"];

		if (f && !f(space, params, props)) {
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
	const object_params& params,
	local_shared_ptr<agent_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(
			type,
			props->isFlying ? PhysicsLayers::flying : PhysicsLayers::onGround,
			props->mass
		),
		props
	),
	props(props),
	level(params.level)
{
	ai_package = params.ai_package.size() > 0 ? params.ai_package : props->ai_package;
}

Agent::~Agent()
{
	allocator_delete(attributeSystem);
}

bullet_attributes Agent::getBulletAttributes(local_shared_ptr<bullet_properties> props) const
{
	bullet_attributes result;

	result.caster = gobject_ref(this);
	result.startRoom = crntRoom;
	result.casterVelocity = getVel();
	result.type = getType() == GType::player ? GType::playerBullet : GType::enemyBullet;
	result.attackDamage = get(Attribute::attack);
	result.bulletSpeed = get(Attribute::bulletSpeed);

	return result;
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
    auto objects = space->scriptVM->_state["objects"];
	auto cls = objects[getClsName()];

    if (cls.valid()) {
		scriptObj = cls(this);
	}
}

void Agent::initFSM()
{
	fsm = make_unique<ai::StateMachine>(this);

	if (ai_package.empty() && type != GType::player) {
		log("%s: no AI package!", toString());
		return;
	}

	if(ai_package.size() > 0) {
		fsm->runScriptPackage(ai_package);
	}
}

void Agent::initAttributes()
{
	attributeSystem = allocator_new<AttributeSystem>(getBaseAttributes());
	applyAttributeEffects(getAttributeUpgrades());
}

void Agent::init()
{
	GObject::init();

	initAttributes();
	initializeRadar();
	initFSM();
}

void Agent::update()
{
	GObject::update();

	if ( (*this)[Attribute::hp] <= 0.0f && (*this)[Attribute::maxHP] >  0.0f) {
		onZeroHP();
	}
	if ((*this)[Attribute::stamina] <= 0.0f && (*this)[Attribute::maxStamina] > 0.0f) {
		fsm->onZeroStamina();
	}

	bool hasFreezeEffect = space->magicEffectSystem->hasScriptedEffect(this, "FreezeStatus");

	if ( (*this)[Attribute::iceDamage] >= AttributeSystem::maxElementDamage && !hasFreezeEffect) {
		applyMagicEffect( app::getEffect("FreezeStatus"), effect_attributes(0.0f, 5.0f));
		modifyAttribute(Attribute::iceDamage, -AttributeSystem::maxElementDamage);
	}
	if ( (*this)[Attribute::sunDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}
	if ( (*this)[Attribute::darknessDamage] >= AttributeSystem::maxElementDamage) {
		applyMagicEffect(app::getEffect("DarknessCurse"), effect_attributes(0.0f, -1.0f));
	}
	if ( (*this)[Attribute::poisonDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}

	if (firePattern) firePattern->update();
	attributeSystem->update(this);
	updateAnimation();
}

void Agent::sendAlert(Player* p)
{
	if(fsm)
		fsm->onAlert(p);
}

void Agent::onDetect(GObject* obj)
{
	if (fsm) {
		fsm->onDetect(obj);
	}
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

bool Agent::applyInitialSpellCost(const spell_cost& cost)
{
	float mpCost = cost.initial_mp;
	float staminaCost = cost.initial_stamina;

	if (mpCost > (*this)[Attribute::mp] || staminaCost > (*this)[Attribute::stamina])
		return false;

	modifyAttribute(Attribute::mp, -mpCost);
	modifyAttribute(Attribute::stamina, -staminaCost);

	return true;
}

bool Agent::applyOngoingSpellCost(const spell_cost& cost)
{
	float mpCost = cost.ongoing_mp * app::params.secondsPerFrame;
	float staminaCost = cost.ongoing_stamina * app::params.secondsPerFrame;

	if (mpCost > (*this)[Attribute::mp] || staminaCost > (*this)[Attribute::stamina]) {
		return false;
	}

	modifyAttribute(Attribute::stamina, -staminaCost);
	modifyAttribute(Attribute::mp, -mpCost);

	return true;
}

AttributeMap Agent::getBaseAttributes() const
{
	return props->attributes;
}

float Agent::get(Attribute id) const
{
	return (*this)[id];
}

void Agent::modifyAttribute(Attribute id, float val)
{
	attributeSystem->modifyAttribute(id, val);
}

void Agent::modifyAttribute(Attribute mod, Attribute addend)
{
	modifyAttribute(mod, addend, 1.0f);
}

void Agent::modifyAttribute(Attribute mod, Attribute addend, float scale)
{
	attributeSystem->modifyAttribute(mod, addend, scale);
}

bool Agent::consume(Attribute attr, float val)
{
	if ((*this)[attr] >= val) {
		modifyAttribute(attr, -val);
		return true;
	}
	return false;
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
	if (get(Attribute::iceSensitivity) >= 1.0f) {
		return GObject::getTraction();
	}
	else
	{
		SpaceFloat traction = GObject::getTraction();

		traction += (1.0 - traction) * (1.0 - get(Attribute::iceSensitivity));

		return traction;
	}
}

SpaceFloat Agent::getMaxSpeed() const
{
	return (*this)[Attribute::maxSpeed];
}

SpaceFloat Agent::getMaxAcceleration() const
{
	return (*this)[Attribute::maxAcceleration];
}

bool Agent::canPlaceBomb(SpaceVect pos)
{
	if (!crntBomb) {
		return false;
	}

	return !isBulletObstacle(pos, crntBomb->dimensions.x);
}

void Agent::setShieldActive(bool v)
{
	shieldActive = v;
}

bool Agent::isShield(Bullet * b)
{
	if (get(Attribute::shieldLevel) <= 0.0f || !shieldActive)
		return false;

	SpaceVect d = -1.0 * b->getVel().normalizeSafe();
	float cost = getShieldCost(d);

	if (cost == -1.0f || cost > get(Attribute::stamina))
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

void Agent::setSprite(shared_ptr<sprite_properties> sprite)
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
	int level = min<int>( (*this)[Attribute::shieldLevel], shieldCosts.size()+1);

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
		modifyAttribute(Attribute::stress, Attribute::stressFromHits, damage.mag);
		hit(damage, n);
		if(fsm) fsm->onBulletHit(b);
	}
	else {
		modifyAttribute(Attribute::stress, Attribute::stressFromBlocks, damage.mag);
		if(fsm) fsm->onBulletBlock(b);
	}
}

void Agent::onTouchAgent(Agent* other)
{
	other->hit(touchEffect(), ai::directionToTarget(this, other->getPos()));
}

void Agent::onEndTouchAgent(Agent* other)
{
}

bool Agent::hit(DamageInfo damage, SpaceVect n)
{
	if (attributeSystem->isNonzero(Attribute::hitProtection) || damage.mag == 0.0f)
		return false;

	if (dynamic_cast<Enemy*>(this)) {
		damage.mag /= app::params.difficultyScale;
	}
	else if (dynamic_cast<Player*>(this)) {
		damage.mag *= app::params.difficultyScale;
	}

	float hp = attributeSystem->applyDamage(damage);

	if (!damage.damageOverTime && hp > 0.0f) {
		space->addGraphicsAction(&graphics_context::createDamageIndicator, hp, getPos());
	}

	SpaceVect knockback = n * damage.knockback;

	if (!knockback.isZero() ) {
		applyImpulse(knockback);
	}

	return true;
}

void Agent::applyAttributeEffects(AttributeMap attributeEffect)
{
	attributeSystem->apply(attributeEffect);
}

DamageInfo Agent::touchEffect() const
{
	return DamageInfo();
}

SpaceFloat Agent::getSensedObjectDistance(GType type) const {
	return radar->getSensedObjectDistance(type);
}

void Agent::initializeRadar()
{
	auto attr = sensor_attributes{
		getRadarRadius(),
		getDefaultFovAngle(),
		hasEssenceRadar()
	};

	if (attr.radius <= 0.0) {
		log("%s has zero radius", getName());
		return;
	}

	radar = allocator_new<RadarSensor>(
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
		allocator_delete(radar);
	}
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
