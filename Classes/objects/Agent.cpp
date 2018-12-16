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
#include "GSpace.hpp"
#include "MagicEffect.hpp"

Agent::Agent(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	PatchConSprite(args),
	StateMachineObject(args),
	RegisterUpdate<Agent>(this),
	RegisterInit<Agent>(this)
{}

void Agent::init()
{
	initStateMachine(fsm);

	attributeSystem = getBaseAttributes();

	attributeSystem.setFullHP();
	attributeSystem.setFullPower();
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

	if (attributeSystem.getAdjustedValue(Attribute::iceDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::iceSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::iceDamage, -App::secondsPerFrame);
	}
	if (attributeSystem.getAdjustedValue(Attribute::sunDamage) > 0 && attributeSystem.getAdjustedValue(Attribute::sunSensitivity) != 0) {
		attributeSystem.modifyAttribute(Attribute::sunDamage, -App::secondsPerFrame);
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

	if (_enemy)
		_enemy->runDamageFlicker();
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
