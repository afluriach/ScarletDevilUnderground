//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "RadarSensor.hpp"

MagicEffect::MagicEffect(GObject* target, float magnitude, float length, effect_flags _flags) :
target(target),
length(length),
magnitude(magnitude),
_flags(_flags),
crntState(state::created)
{
	if (_flags == effect_flags::none) {
		log("Warning, empty MagicEffect created.");
	}

	id = getSpace()->magicEffectSystem->nextID++;
}

GSpace* MagicEffect::getSpace() const {
	return target->space;
}

bool MagicEffect::isImmediate() const
{
	return bitwise_and_bool(_flags, effect_flags::immediate);
}

bool MagicEffect::isTimed() const
{
	return bitwise_and_bool(_flags, effect_flags::timed);
}

bool MagicEffect::isActive() const
{
	return bitwise_and_bool(_flags, effect_flags::active);
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

ScriptedMagicEffect::ScriptedMagicEffect(GObject* agent, float magnitude, float length, string clsName) :
	MagicEffect(agent, magnitude, length, getFlags(clsName)),
	clsName(clsName)
{
	auto cls = GSpace::scriptVM->_state["effects"][clsName];
	MagicEffect* super_this = this;

	if (!cls) {
		log("ScriptedMagicEffect: %s not found", clsName);
	}
	else {
		obj = cls(super_this, agent, magnitude, length);
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

RadiusEffect::RadiusEffect(GObject* target, SpaceFloat radius, GType type) :
	MagicEffect(target, -0.0f, -1.0f, enum_bitwise_or(effect_flags, indefinite, active)),
	radius(radius),
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

	sensor = new RadarSensor(
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
		delete sensor;
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

DamageRadiusEffect::DamageRadiusEffect(GObject* agent, DamageInfo damage, SpaceFloat radius, GType type) :
	RadiusEffect(agent, radius, type),
	damage(damage)
{}

void DamageRadiusEffect::onHit(GObject* target)
{
	if (auto agent = dynamic_cast<Agent*>(target)) {
		agent->hit(damage);
	}
}
