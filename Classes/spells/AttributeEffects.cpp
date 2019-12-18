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
	ImmediateMagicEffect(target, magnitude),
	attr(attr)
{
	
}

void RestoreAttribute::init()
{
	if (auto _agent = dynamic_cast<Agent*>(agent)) {
		_agent->modifyAttribute(attr, magnitude);
	}
}
