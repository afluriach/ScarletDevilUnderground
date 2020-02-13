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
#include "physics_context.hpp"
#include "Player.hpp"
#include "Torch.hpp"

EffectArea::EffectArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(AreaSensor)
{
}

void EffectArea::beginContact(GObject* obj)
{
	AreaSensor::beginContact(obj);
	targets.insert(obj);
}

void EffectArea::endContact(GObject* obj)
{
	AreaSensor::endContact(obj);
	targets.erase(obj);
}

void EffectArea::update()
{
	//shouldn't actually be necessary
	//GObject::update();

	for ( auto target : targets){
		target->hit(getDamageInfo(), SpaceVect::zero);
	}
}

SunArea::SunArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(EffectArea)
{
}

boost::shared_ptr<LightArea> SunArea::getLightSource() const
{
	return AmbientLightArea::create(getDimensions(), toColor4F(Color3B(192, 192, 82)));
}

GraphicsLayer SunArea::sceneLayer() const{
	return GraphicsLayer::overhead;
}

DamageInfo SunArea::getDamageInfo() const {
	return DamageInfo(5.0f, DamageType::effectArea, Attribute::sunDamage, 0.0f);
}

DarknessArea::DarknessArea(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(EffectArea)
{
}

void DarknessArea::init()
{
	GObject::init();

	torches = space->physicsContext->rectangleQueryByType<Torch>(
		getPos(),
		getDimensions(),
		GType::environment,
		PhysicsLayers::all
	);
}

void DarknessArea::update()
{
	EffectArea::update();

	active = true;
	for (Torch* t : torches) {
		if (t->getActive()) {
			active = false;
			break;
		}
	}
}

DamageInfo DarknessArea::getDamageInfo() const {
	return
		active ? 
		DamageInfo(5.0f, DamageType::effectArea, Attribute::darknessDamage, 0.0f) :
		DamageInfo()
	;
}
