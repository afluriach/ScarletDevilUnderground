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
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "RadarSensor.hpp"

MagicEffect::MagicEffect(GObject* agent, float length, float magnitude, flag_bits _flags) :
agent(agent),
length(length),
magnitude(magnitude),
_flags(_flags),
crntState(state::created)
{
	if (!_flags.any()) {
		log("Warning, empty MagicEffect created.");
	}
}

GSpace* MagicEffect::getSpace() const {
	return agent->space;
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
	getSpace()->magicEffectSystem->removeEffect(this);
}

RadiusEffect::RadiusEffect(GObject* agent, SpaceFloat radius, GType type) :
	MagicEffect(agent, -1.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
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
		agent,
		attr,
		bind(&RadiusEffect::onContact, this, placeholders::_1),
		bind(&RadiusEffect::onEndContact, this, placeholders::_1)
	);

	agent->space->insertSensor(sensor);
}

void RadiusEffect::update()
{
	for (auto obj : contacts)
		onHit(obj);
}

void RadiusEffect::end()
{
	if (sensor) {
		agent->space->removeSensor(sensor);
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
