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

bool RestoreAttribute::canApply(GObject* target, effect_attributes attrs, Attribute attr)
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		return _agent->getAttributeSystem()->canApplyAttribute(attr, attrs.magnitude);
	}
	else {
		return false;
	}
}

RestoreAttribute::RestoreAttribute(effect_params params, Attribute attr) :
	MagicEffect(params),
	attr(attr)
{
	
}

void RestoreAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}

FortifyAttribute::FortifyAttribute(effect_params params, Attribute attr) :
	MagicEffect(params),
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

DrainFromMovement::DrainFromMovement(effect_params params, Attribute attr) :
	MagicEffect(params),
	attr(attr)
{
	_ratio = -1.0f * app::params.secondsPerFrame * magnitude;
	agent = dynamic_cast<Agent*>(target);
}

void DrainFromMovement::update()
{
	if (agent)
		agent->modifyAttribute(attr, Attribute::currentSpeed, _ratio);
}

SetBoolAttribute::SetBoolAttribute(effect_params params, Attribute attr) :
	MagicEffect(params),
	attr(attr)
{
}

void SetBoolAttribute::init()
{
	auto agent = dynamic_cast<Agent*>(target);
	agent->increment(attr);
}

void SetBoolAttribute::end()
{
	auto agent = dynamic_cast<Agent*>(target);
	agent->decrement(attr);
}
