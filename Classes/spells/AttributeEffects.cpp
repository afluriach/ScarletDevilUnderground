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

bool RestoreAttribute::canApply(GObject* target, float magnitude, Attribute attr)
{
	if (auto _agent = dynamic_cast<Agent*>(target)) {
		return _agent->getAttributeSystem()->canApplyAttribute(attr, magnitude);
	}
	else {
		return false;
	}
}

RestoreAttribute::RestoreAttribute(GObject* target, float magnitude, Attribute attr) :
	MagicEffect(target, 0.0f, magnitude, immediate),
	attr(attr)
{
	
}

void RestoreAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(agent)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}

FortifyAttribute::FortifyAttribute(GObject* target, float magnitude, float length, Attribute attr) :
	MagicEffect(target, length, magnitude, make_enum_bitfield(flags::timed)),
	attr(attr)
{
}

void FortifyAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(agent)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}

void FortifyAttribute::end()
{
	if (auto _agent = dynamic_cast<Agent*>(agent)) {
		_agent->modifyAttribute(attr, -magnitude);
	}
}
