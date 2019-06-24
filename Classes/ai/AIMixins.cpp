//
//  AIMixins.cpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "AIMixins.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "physics_context.hpp"
#include "RadarSensor.hpp"

RadarObject::RadarObject()
{}

void RadarObject::initializeRadar(GSpace& space)
{
	auto attr = sensor_attributes {
		getRadarRadius(),
		getDefaultFovAngle(),
		getRadarType(),
		hasEssenceRadar()
	};

	if (attr.radius <= 0.0) {
		log("%s has zero radius", getName());
		return;
	}

	radar = new RadarSensor(
		this,
		attr,
		bind(&RadarObject::onDetect, this, placeholders::_1),
		bind(&RadarObject::onEndDetect, this, placeholders::_1)
	);
}

void RadarObject::_update()
{
	if (radar)
		radar->update();
}

void RadarObject::removePhysicsObjects()
{
	GObject::removePhysicsObjects();

	if (radar) delete radar;
}
