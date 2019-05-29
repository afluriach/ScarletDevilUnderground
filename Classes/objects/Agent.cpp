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
#include "app_constants.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "FirePattern.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "MagicEffect.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"

const Color4F Agent::bodyOutlineColor = hsva4F(270.0f, 0.2f, 0.7f, 0.667f);
const Color4F Agent::shieldConeColor = Color4F(.37f, .56f, .57f, 0.5f);
const float Agent::bodyOutlineWidth = 4.0f;

Agent::Agent(GSpace* space, ObjectIDType id, const string& name, const SpaceVect& pos, Direction d) :
	GObject(space, id, name,pos, dirToPhysicsAngle(d)),
	PatchConSprite(d)
{
	multiInit.insertWithOrder(wrap_method(Agent, initFSM, this), to_int(GObject::initOrder::initFSM));
	multiInit.insertWithOrder(wrap_method(Agent, initAttributes, this), to_int(GObject::initOrder::loadAttributes));
}

Agent::Agent(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	PatchConSprite(args),
	StateMachineObject(args)
{
	multiInit.insertWithOrder(wrap_method(Agent, initFSM, this), to_int(GObject::initOrder::initFSM));
	multiInit.insertWithOrder(wrap_method(Agent, initAttributes, this), to_int(GObject::initOrder::loadAttributes));
}

void Agent::initFSM()
{
	initStateMachine(fsm);
}

void Agent::initAttributes()
{
	attributeSystem = getBaseAttributes();
	applyAttributeEffects(getAttributeUpgrades());

	attributeSystem.setFullHP();
	attributeSystem.setFullMP();
	attributeSystem.setFullStamina();

	space->graphicsNodeAction(
		&AgentBodyShader::setShieldLevel,
		agentOverlay,
		getAttribute(Attribute::shieldLevel)
	);
}

void Agent::update()
{
	GObject::update();
	RadarObject::_update();
	StateMachineObject::_update();
	PatchConSprite::_update();

	if (attributeSystem[Attribute::hp] <= 0.0f && attributeSystem[Attribute::maxHP] >  0.0f) {
		onZeroHP();
	}

	if (attributeSystem[Attribute::iceDamage] >= AttributeSystem::maxElementDamage && !hasMagicEffect<FreezeStatusEffect>()) {
		addMagicEffect(make_shared<FreezeStatusEffect>(this));
		attributeSystem.modifyAttribute(Attribute::iceDamage, -AttributeSystem::maxElementDamage);
	}
	if (attributeSystem[Attribute::sunDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}
	if (attributeSystem[Attribute::darknessDamage] >= AttributeSystem::maxElementDamage) {
		addMagicEffect(make_shared<DarknessCurseEffect>(this));
	}
	if (attributeSystem[Attribute::poisonDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}

	for (auto other : touchTargets) {
		other->hit(touchEffect());
	}

	if (firePattern) firePattern->update();
	attributeSystem.update();
	updateAgentOverlay();
}

bool Agent::isBulletObstacle(SpaceVect pos, SpaceFloat radius)
{
	return space->obstacleRadiusQuery(this, pos, radius, bulletObstacles, PhysicsLayers::ground);
}

void Agent::sendAlert(Player* p)
{
	fsm.onAlert(p);
}

void Agent::onDetect(GObject* obj)
{
	if (obj->getType() == GType::playerBullet) {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromDetects);
	}

	fsm.onDetect(obj);
}

void Agent::onEndDetect(GObject* obj)
{
	fsm.onEndDetect(obj);
}

void Agent::onZeroHP()
{
	space->removeObject(this);
}

void Agent::onRemove()
{
	space->addGraphicsAction(&graphics_context::removeSprite, agentOverlay);
}

bool Agent::cast(shared_ptr<Spell> spell)
{
	if (!spell->getDescriptor()) return GObject::cast(spell);

	float cost = spell->getDescriptor()->getCost();
	SpellCostType costType = spell->getDescriptor()->getCostType();

	if (cost == 0.0f || costType == SpellCostType::none)
		return GObject::cast(spell);

	float mpCost = cost *
		bitwise_and_bool(costType, SpellCostType::mp) *
		bitwise_and_bool(costType, SpellCostType::initial);

	float staminaCost = cost *
		bitwise_and_bool(costType, SpellCostType::stamina) *
		bitwise_and_bool(costType, SpellCostType::initial);

	if (mpCost > attributeSystem[Attribute::mp] || staminaCost > attributeSystem[Attribute::stamina])
		return false;

	attributeSystem.modifyAttribute(Attribute::mp, -mpCost);
	attributeSystem.modifyAttribute(Attribute::stamina, -staminaCost);

	return GObject::cast(spell);
}

void Agent::updateSpells()
{
	GObject::updateSpells();

	if (!crntSpell) {
		attributeSystem.timerDecrement(Attribute::spellCooldown);
		return;
	};
	if (!crntSpell->getDescriptor()) return;

	float cost = crntSpell->getDescriptor()->getCost();
	SpellCostType costType = crntSpell->getDescriptor()->getCostType();

	if (cost == 0.0f || costType == SpellCostType::none) return;

	float mpCost =
		bitwise_and_bool(costType, SpellCostType::mp) *
		bitwise_and_bool(costType, SpellCostType::ongoing) *
		cost * app::params.secondsPerFrame
	;
	float staminaCost =
		bitwise_and_bool(costType, SpellCostType::stamina) *
		bitwise_and_bool(costType, SpellCostType::ongoing) *
		cost * app::params.secondsPerFrame
	;

	if (mpCost > attributeSystem[Attribute::mp] || staminaCost > attributeSystem[Attribute::stamina]) {
		stopSpell();
	}

	attributeSystem.modifyAttribute(Attribute::stamina, -staminaCost);
	attributeSystem.modifyAttribute(Attribute::mp, -mpCost);
}

float Agent::getAttribute(Attribute id) const
{
	return attributeSystem[id];
}

void Agent::modifyAttribute(Attribute id, float val)
{
	attributeSystem.modifyAttribute(id, val);
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
	return attributeSystem[Attribute::speed];
}

SpaceFloat Agent::getMaxAcceleration() const
{
	return attributeSystem[Attribute::acceleration];
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

void Agent::setShieldActive(bool v)
{
	space->graphicsNodeAction(&Node::setVisible, agentOverlay, v);
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
	PatchConSprite::initializeGraphics();

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
	space->graphicsNodeAction(&Node::setVisible, agentOverlay, shieldActive);
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

void Agent::onBulletCollide(Bullet* b)
{
	DamageInfo damage = b->getScaledDamageInfo();

	if (!isShield(b)) {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromHits, damage.mag);
		hit(damage);
		fsm.onBulletHit(b);
	}
	else {
		attributeSystem.modifyAttribute(Attribute::stress, Attribute::stressFromBlocks, damage.mag);
		fsm.onBulletBlock(b);
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

bool Agent::hit(DamageInfo damage)
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

	return true;
}

bool Agent::canApplyAttributeEffects(AttributeMap attributeEffect)
{
	for (auto& entry : attributeEffect)
	{
		if (!attributeSystem.canApplyAttribute(entry.first, entry.second)) {
			return false;
		}
	}
	return true;
}


void Agent::applyAttributeEffects(AttributeMap attributeEffect)
{
	attributeSystem.apply(attributeEffect);
}

DamageInfo Agent::touchEffect() const
{
	if (attributeSystem.isNonzero(Attribute::touchDamage))
		return DamageInfo{ attributeSystem[Attribute::touchDamage] , Attribute::end, DamageType::touch };
	else
		return DamageInfo{};
}

void Agent::updateAgentOverlay()
{
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
}

const AttributeMap GenericAgent::baseAttributes = {
	{Attribute::speed, 1.5f },
	{Attribute::acceleration, 6.0f }
};

GenericAgent::GenericAgent(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{
	spriteName = args.at("sprite").asString();
}

void GenericAgent::initStateMachine(ai::StateMachine& sm)
{
	auto wanderThread = make_shared<ai::Thread>(
		make_shared<ai::Wander>(1.0, 3.0, 2.0, 4.0),
		&fsm,
		0,
		make_enum_bitfield(ai::ResourceLock::movement)
	);

	sm.addDetectFunction(
		GType::player,
		[=](ai::StateMachine& sm, GObject* target) -> void {
			 sm.addThread(make_shared<ai::Flee>(target, 3.0f), 1);
		}
	);

	sm.addEndDetectFunction(
		GType::player,
		[=](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);

	wanderThread->setResetOnBlock(true);    
	fsm.addThread(wanderThread);
}
