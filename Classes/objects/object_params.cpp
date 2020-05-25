//
//  object_params.cpp
//  Koumachika
//
//  Created by Toni on 6/19/19.
//
//

#include "Prefix.h"

//#include "object_params.hpp"
#include "value_map.hpp"

object_params::object_params(
	const SpaceVect& pos,
	SpaceFloat angle
) :
	pos(pos),
	angle(angle)
{
}

object_params::object_params(
	const SpaceVect& pos,
	const SpaceVect& vel,
	SpaceFloat angle
) : 
	pos(pos),
	vel(vel),
	angle(angle)
{
}

object_params::object_params(const SpaceRect& rect) :
	pos(rect.center),
	dimensions(rect.dimensions)
{
}

object_params::object_params(const ValueMap& args) :
	object_params(
		getObjectPos(args),
		float_pi * 0.5
	)
{
	name = getStringOrDefault(args, "name", "");
	ai_package = getStringOrDefault(args, "ai_package", "");

	level = getIntOrDefault(args, "level", 0);

	dimensions = getObjectDimensions(args);

	hidden = getBoolOrDefault(args, "hidden", false);
	active = getBoolOrDefault(args, "active", false);

	Direction dir = getDirectionOrDefault(args, Direction::none);
	if (dir != Direction::none) {
		angle = dirToPhysicsAngle(dir);
	}
}

physics_params::physics_params(GType type, PhysicsLayers layers, SpaceFloat mass, bool sensor) :
	type(type),
	layers(layers),
	mass(mass),
	sensor(sensor)
{}

bullet_attributes bullet_attributes::getDefault()
{
	return bullet_attributes{
		SpaceVect::zero,
		nullptr,
		GType::enemyBullet,
		nullptr,
		0,
		1.0f,
		1.0f,
		1.0f
	};
}

bullet_properties bullet_properties::clone() {
	return *this;
}
