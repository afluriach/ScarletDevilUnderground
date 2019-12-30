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
	GSpace* space,
	ObjectIDType id,
	const string& name,
	const SpaceVect& pos,
	SpaceFloat angle
) :
	space(space),
	id(id),
	name(name),
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

object_params::object_params(GSpace* space, ObjectIDType id, const ValueMap& args, bool rotateUp) :
	object_params(
		space,
		id,
		getStringOrDefault(args, "name", ""),
		getObjectPos(args),
		rotateUp ? float_pi * 0.5 : 0.0
	)
{
	hidden = getBoolOrDefault(args, "hidden", false);
	name = getStringOrDefault(args, "name", "");

	Direction dir = getDirectionOrDefault(args, Direction::none);
	if (dir != Direction::none) {
		angle = dirToPhysicsAngle(dir);
	}
}

physics_params::physics_params(GType type, PhysicsLayers layers, SpaceFloat radius, SpaceFloat mass, bool sensor) :
	type(type),
	layers(layers),
	dimensions(SpaceVect(radius,0.0)),
	mass(mass),
	sensor(sensor)
{}

physics_params::physics_params(GType type, PhysicsLayers layers, SpaceVect dimensions, SpaceFloat mass, bool sensor) :
	type(type),
	layers(layers),
	dimensions(dimensions),
	mass(mass),
	sensor(sensor)
{}

physics_params::physics_params(GType type, PhysicsLayers layers, const ValueMap& args, SpaceFloat mass, bool sensor)
{
	this->type = type;
	this->layers = layers;
	dimensions = getObjectDimensions(args);
	this->mass = getFloatOrDefault(args, "mass", mass);
	this->sensor = sensor;
}

bullet_attributes bullet_attributes::getDefault()
{
	return bullet_attributes{
		SpaceVect::zero,
		nullptr,
		GType::enemyBullet,
		nullptr,
		1.0f,
		1.0f,
		1.0f
	};
}

bullet_properties bullet_properties::clone() {
	return *this;
}
