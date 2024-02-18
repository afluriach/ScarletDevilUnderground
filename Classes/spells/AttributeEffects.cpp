//
//  AttributeEffects.cpp
//  Koumachika
//
//  Created by Toni on 12/18/19.
//
//

#include "Prefix.h"

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
	AgentEffect(params),
	attr(attr)
{
	
}

void RestoreAttribute::init()
{
	agent->modifyAttribute(attr, magnitude);
}

FortifyAttribute::FortifyAttribute(effect_params params, Attribute attr) :
	AgentEffect(params),
	attr(attr)
{
}

void FortifyAttribute::init()
{
	agent->modifyAttribute(attr, magnitude);
}

void FortifyAttribute::end()
{
	agent->modifyAttribute(attr, -magnitude);
}

ScaleAttributes::ScaleAttributes(effect_params params, AttributeMap scales) :
	AgentEffect(params),
	scales(scales)
{
}

void ScaleAttributes::init()
{
	for(auto entry : scales){
		float differential = (*agent)[entry.first] * ( 1.0f - entry.second);
		agent->setAttribute(entry.first, (*agent)[entry.first] * entry.second);
		diff.insert(pair(entry.first, differential));
	}
}

void ScaleAttributes::end()
{
	for(auto entry : diff){
		agent->modifyAttribute(entry.first, entry.second);
	}
}

DrainFromMovement::DrainFromMovement(effect_params params, Attribute attr) :
	AgentEffect(params),
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
	AgentEffect(params),
	attr(attr)
{
}

void SetBoolAttribute::init()
{
	agent->increment(attr);
}

void SetBoolAttribute::end()
{
	agent->decrement(attr);
}

ApplyDamage::ApplyDamage(effect_params params, Element element) :
	AgentEffect(params),
	element(element)
{
}

void ApplyDamage::init()
{
	DamageInfo damage(magnitude, damageType, element, 0.0f);
	target->hit(damage, SpaceVect::zero);
}

DamageOverTime::DamageOverTime(effect_params params, Element element) :
	AgentEffect(params),
	element(element)
{
}

void DamageOverTime::update()
{
	DamageInfo damage(magnitude, damageType, element, 0.0f);
	damage.damageOverTime = true;

	target->hit(damage, SpaceVect::zero);
}
