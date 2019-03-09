//
//  Agent.cpp
//  Koumachika
//
//  Created by Toni on 12/16/17.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "FirePattern.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "MagicEffect.hpp"

Agent::Agent(GSpace* space, ObjectIDType id, const string& name, const SpaceVect& pos, Direction d) :
	GObject(space, id, name,pos, dirToPhysicsAngle(d)),
	RegisterUpdate<Agent>(this),
	PatchConSprite(d)
{
	multiInit.insertWithOrder(wrap_method(Agent, initFSM, this), to_int(GObject::initOrder::initFSM));
	multiInit.insertWithOrder(wrap_method(Agent, initAttributes, this), to_int(GObject::initOrder::loadAttributes));
}

Agent::Agent(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	PatchConSprite(args),
	StateMachineObject(args),
	RegisterUpdate<Agent>(this)
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
}

void Agent::update()
{
	if (attributeSystem[Attribute::hp] <= 0 && getMaxHealth() != 0) {
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
		onZeroHP();
	}
	if (attributeSystem[Attribute::poisonDamage] >= AttributeSystem::maxElementDamage) {
		onZeroHP();
	}

	attributeSystem.update();

	if (firePattern) {
		firePattern->update();
	}
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

bool Agent::isShield(Bullet * b)
{
	if (getAttribute(Attribute::shieldLevel) <= 0.0f || getAttribute(Attribute::shieldActive) <= 0.0f)
		return false;

	SpaceVect d = -1.0 * b->getVel().normalizeSafe();
	float cost = getShieldCost(d);

	if (cost == -1.0f || cost > getAttribute(Attribute::stamina))
		return false;

	modifyAttribute(Attribute::stamina, -cost);
	return true;
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
	if (auto _eb = dynamic_cast<EnemyBullet*>(b)){
		_eb->invalidateGraze();
	}

	if (!isShield(b)) {
		hit(AttributeSystem::scale(b->getAttributeEffect(), b->agentAttackMultiplier), b->getMagicEffect(this));
	}
	fsm.onBulletHit(b);
}

void Agent::hit(AttributeMap attributeEffect, shared_ptr<MagicEffect> effect)
{
	if (effect)
		addMagicEffect(effect);

	for (auto& entry : attributeEffect)
	{
		attributeSystem.modifyAttribute(entry.first, entry.second);
	}

	Enemy* _enemy = dynamic_cast<Enemy*>(this);

	if (_enemy) {
		_enemy->runDamageFlicker();
		App::playSoundSpatial("sfx/enemy_damage.wav", toFmod(getPos()), toFmod(getVel()));
	}

	auto hp_it = attributeEffect.find(Attribute::hp);
	float hp = hp_it == attributeEffect.end() ? 0.0f : hp_it->second;

	space->createDamageIndicator(-hp, getPos());
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
	for (auto& entry : attributeEffect)
	{
		attributeSystem.modifyAttribute(entry.first, entry.second);
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
