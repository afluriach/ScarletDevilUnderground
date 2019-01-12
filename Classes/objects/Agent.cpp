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
#include "Enemy.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "MagicEffect.hpp"

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
	attributeSystem.setStartPower();
	attributeSystem.setStartMP();
}

void Agent::update()
{
	if (attributeSystem.getAdjustedValue(Attribute::hp) <= 0 && getMaxHealth() != 0) {
		onZeroHP();
	}

	if (attributeSystem.getAdjustedValue(Attribute::iceDamage) >= 100.0f && !hasMagicEffect<FreezeStatusEffect>()) {
		addMagicEffect(make_shared<FreezeStatusEffect>(this));
	}
	if (attributeSystem.getAdjustedValue(Attribute::sunDamage) >= 100.0f ) {
		onZeroHP();
	}
	if (attributeSystem.getAdjustedValue(Attribute::poisonDamage) >= 100.0f) {
		onZeroHP();
	}

	if (attributeSystem.getAdjustedValue(Attribute::iceDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::iceSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::iceDamage, -App::secondsPerFrame);
	}
	if (attributeSystem.getAdjustedValue(Attribute::sunDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::sunSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::sunDamage, -App::secondsPerFrame);
	}
	if (attributeSystem.getAdjustedValue(Attribute::poisonDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::poisonSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::poisonDamage, -App::secondsPerFrame);
	}
	if (attributeSystem.getAdjustedValue(Attribute::slimeDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::slimeSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::slimeDamage, -App::secondsPerFrame);
	}

}

void Agent::onDetect(GObject* obj)
{
	fsm.onDetect(obj);
	RadarObject::onDetect(obj);
}

void Agent::onEndDetect(GObject* obj)
{
	fsm.onEndDetect(obj);
	RadarObject::onEndDetect(obj);
}

void Agent::onZeroHP()
{
	space->removeObject(this);
}

float Agent::getAttribute(Attribute id) const
{
	return attributeSystem.getAdjustedValue(id);
}

void Agent::modifyAttribute(Attribute id, float val) 
{
	attributeSystem.modifyAttribute(id,val);
}

float Agent::_getAttribute(int id) const
{
	return getAttribute(static_cast<Attribute>(id));
}

void Agent::_modifyAttribute(int id, float val)
{
	attributeSystem.modifyAttribute(static_cast<Attribute>(id), val);
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
	return attributeSystem.getAdjustedValue(Attribute::speed);
}

SpaceFloat Agent::getMaxAcceleration() const
{
	return attributeSystem.getAdjustedValue(Attribute::acceleration);
}

float Agent::getMaxHealth() const
{
	return attributeSystem.getAdjustedValue(Attribute::maxHP);
}

float Agent::getMaxPower() const
{
	return attributeSystem.getAdjustedValue(Attribute::maxPower);
}

int Agent::getHealth()
{
	return attributeSystem.getAdjustedValue(Attribute::hp);
}

int Agent::getPower()
{
	return attributeSystem.getAdjustedValue(Attribute::power);
}

int Agent::getMagic()
{
	return attributeSystem.getAdjustedValue(Attribute::mp);
}


bool Agent::consumePower(int val)
{
	if (getPower() >= val) {
		attributeSystem.modifyAttribute(Attribute::power, -val);
		return true;
	}
	return false;
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
		App::playSoundSpatial("sfx/enemy_damage.wav", getPos(), getVel());
	}
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
    make_shared<ai::Wander>(),
    &fsm,
    0,
    make_enum_bitfield(ai::ResourceLock::movement)
);
    
auto detectThread = make_shared<ai::Thread>(
    make_shared<ai::Detect>(
        "player",
        [=](GObject* target) -> shared_ptr<ai::Function> {
            return make_shared<ai::Flee>(target, 3.0f);
        }
    ),
    &fsm,
    1,
    bitset<ai::lockCount>()
);
    
wanderThread->setResetOnBlock(true);
    
fsm.addThread(wanderThread);
fsm.addThread(detectThread);
}
