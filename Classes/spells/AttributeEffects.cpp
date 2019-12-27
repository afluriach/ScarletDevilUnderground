//
//  AttributeEffects.cpp
//  Koumachika
//
//  Created by Toni on 12/18/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AttributeEffects.hpp"

bool RestoreAttribute::canApply(GObject* target, float magnitude, float length, Attribute attr)
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		return _agent->getAttributeSystem()->canApplyAttribute(attr, magnitude);
	}
	else {
		return false;
	}
}

RestoreAttribute::RestoreAttribute(effect_params params, float magnitude, float length, Attribute attr) :
	MagicEffect(params, magnitude, length, effect_flags::immediate),
	attr(attr)
{
	
}

void RestoreAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}

FortifyAttribute::FortifyAttribute(effect_params params, float magnitude, float length, Attribute attr) :
	MagicEffect(params, magnitude, length, effect_flags::timed),
	attr(attr)
{
}

void FortifyAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}

void FortifyAttribute::end()
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		_agent->modifyAttribute(attr, -magnitude);
	}
}

DrainFromMovement::DrainFromMovement(effect_params params, float magnitude, float length, Attribute attr) :
	MagicEffect(params, magnitude, length, enum_bitwise_or(effect_flags, indefinite, active)),
	attr(attr)
{
	_ratio = -1.0f * app::params.secondsPerFrame * magnitude;
	agent = dynamic_cast<Agent*>(target);
}

void DrainFromMovement::update()
{
	if (agent)
		agent->modifyAttribute(attr, _ratio * agent->getAttribute(Attribute::currentSpeed));
}
