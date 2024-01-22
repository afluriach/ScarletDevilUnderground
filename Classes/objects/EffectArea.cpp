//
//  EffectArea.cpp
//  Koumachika
//
//  Created by Toni on 12/14/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EffectArea.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"
#include "physics_context.hpp"
#include "Player.hpp"

effectarea_properties::effectarea_properties()
{}

effectarea_properties::~effectarea_properties()
{}

EffectArea::EffectArea(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<effectarea_properties> props
) :
	AreaSensor(space,id,params, props),
	props(props)
{
	attr = effect_attributes(props->magnitude, -1.0f, 0.0f, DamageType::effectArea);
}

void EffectArea::beginContact(GObject* obj)
{
	AreaSensor::beginContact(obj);
		
	if (props->effect->canApply(obj, attr)) {
		unsigned int effectID = obj->applyMagicEffect(props->effect, attr);
		activeEffects.insert_or_assign(obj, effectID);
	}
}

void EffectArea::endContact(GObject* obj)
{
	AreaSensor::endContact(obj);

	auto it = activeEffects.find(obj);
	if (it != activeEffects.end()) {
		space->magicEffectSystem->removeEffect(it->second);
		activeEffects.erase(it);
	}
}
