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

const MagicEffect::flag_bits MagicEffect::immediate = make_enum_bitfield(flags::immediate);

MagicEffect::MagicEffect(GObject* target, float length, float magnitude, flag_bits _flags) :
target(target),
length(length),
magnitude(magnitude),
_flags(_flags),
crntState(state::created)
{
	if (!_flags.any()) {
		log("Warning, empty MagicEffect created.");
	}

	id = getSpace()->magicEffectSystem->nextID++;
}

GSpace* MagicEffect::getSpace() const {
	return target->space;
}

bool MagicEffect::isImmediate() const
{
	return (_flags & make_enum_bitfield(flags::immediate)).any();
}

bool MagicEffect::isTimed() const
{
	return (_flags & make_enum_bitfield(flags::timed)).any();
}

bool MagicEffect::isActive() const
{
	return (_flags & make_enum_bitfield(flags::active)).any();
}

void MagicEffect::remove()
{
	getSpace()->magicEffectSystem->removeEffect(id);
}

MagicEffect::flag_bits ScriptedMagicEffect::getFlags(string clsName)
{
	sol::table cls = GSpace::scriptVM->_state["effects"][clsName];

	if (!cls) {
		log("ScriptedMagicEffect::getFlags: %s does not exist", clsName);
		return flag_bits();
	}

	sol::function getFlags = cls["getFlags"];
	return getFlags ? getFlags() : flag_bits();
}

ScriptedMagicEffect::ScriptedMagicEffect(string clsName, GObject* agent) :
	ScriptedMagicEffect(clsName, agent, 0.0f, 0.0f)
{
}

ScriptedMagicEffect::ScriptedMagicEffect(string clsName, GObject* agent, float length, float magnitude) :
	MagicEffect(agent, length, magnitude, getFlags(clsName)),
	clsName(clsName)
{
	auto cls = GSpace::scriptVM->_state["effects"][clsName];
	MagicEffect* super_this = this;

	if (!cls) {
		log("ScriptedMagicEffect: %s not found", clsName);
	}
	else {
		obj = cls(super_this, agent, length, magnitude);
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
	MagicEffect(target, -1.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
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
