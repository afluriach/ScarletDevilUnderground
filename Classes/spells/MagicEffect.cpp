//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "RadarSensor.hpp"

MagicEffect::MagicEffect(effect_params params) :
target(params.target),
agent(dynamic_cast<Agent*>(params.target)),
length(params.attr.length),
magnitude(params.attr.magnitude),
_flags(params.flags),
id(params.id),
desc(params.desc),
damageType(params.attr.type),
crntState(state::created)
{
	if (_flags == effect_flags::none) {
		log("Warning, empty MagicEffect created.");
	}
}

GSpace* MagicEffect::getSpace() const {
	return target->space;
}

bool MagicEffect::isImmediate() const
{
	return bitwise_and_bool(_flags, effect_flags::immediate);
}

bool MagicEffect::isDurable() const
{
	return bitwise_and_bool(_flags, effect_flags::durable);
}

bool MagicEffect::isActive() const
{
	return bitwise_and_bool(_flags, effect_flags::active);
}

bool MagicEffect::isAgentEffect() const
{
	return bitwise_and_bool(_flags, effect_flags::agent);
}

void MagicEffect::remove()
{
	getSpace()->magicEffectSystem->removeEffect(id);
}

effect_flags ScriptedMagicEffect::getFlags(string clsName)
{
	sol::table cls = GSpace::scriptVM->_state["effects"][clsName];

	if (!cls) {
		log("ScriptedMagicEffect::getFlags: %s does not exist", clsName);
		return effect_flags::none;
	}

	sol::object obj = cls["flags"];

	return obj ? obj.as<effect_flags>() : effect_flags::none;
}

ScriptedMagicEffect::ScriptedMagicEffect(effect_params params, string clsName) :
	MagicEffect(params),
	clsName(clsName)
{
	auto cls = GSpace::scriptVM->_state["effects"][clsName];
	MagicEffect* super_this = this;

	if (!cls) {
		log("ScriptedMagicEffect: %s not found", clsName);
	}
	else {
		obj = cls(super_this);
	}
}

void ScriptedMagicEffect::init()
{
	if (obj) {
		sol::function f = obj["onEnter"];
		if (f) f(obj);
	}
}

void ScriptedMagicEffect::update()
{
	if (obj) {
		sol::function f = obj["update"];
		if (f) f(obj);
	}
}

void ScriptedMagicEffect::end()
{
	if (obj) {
		sol::function f = obj["onExit"];
		if (f) f(obj);
	}
}

RadiusEffect::RadiusEffect(effect_params params, GType type) :
	MagicEffect(params),
	radius(params.attr.radius),
	type(type)
{}

void RadiusEffect::init()
{
	sensor_attributes attr = {
		radius,
		0.0,
		GType::enemySensor,
		true
	};

	sensor = allocator_new<RadarSensor>(
		target,
		attr,
		bind(&RadiusEffect::onContact, this, placeholders::_1),
		bind(&RadiusEffect::onEndContact, this, placeholders::_1)
	);

	target->space->insertSensor(sensor);
}

void RadiusEffect::update()
{
	for (auto obj : contacts)
		onHit(obj);
}

void RadiusEffect::end()
{
	if (sensor) {
		target->space->removeSensor(sensor);
		allocator_delete(sensor);
	}
}

void RadiusEffect::onContact(GObject* obj)
{
	if(obj->getType() == type)
		contacts.insert(obj);
}

void RadiusEffect::onEndContact(GObject* obj)
{
	if (obj->getType() == type)
		contacts.erase(obj);
}

DamageRadiusEffect::DamageRadiusEffect(effect_params params, DamageInfo damage, GType type) :
	RadiusEffect(params, type),
	damage(damage)
{}

void DamageRadiusEffect::onHit(GObject* target)
{
	if (auto agent = dynamic_cast<Agent*>(target)) {
		agent->hit(damage, ai::directionToTarget(sensor->getPos(), target->getPos()) );
	}
}
