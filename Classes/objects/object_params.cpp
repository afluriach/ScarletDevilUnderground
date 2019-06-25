//
//  object_params.cpp
//  Koumachika
//
//  Created by Toni on 6/19/19.
//
//

#include "Prefix.h"

#include "object_params.hpp"
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

object_params::object_params(GSpace* space, ObjectIDType id, const ValueMap& args) :
	object_params(
		space,
		id,
		getStringOrDefault(args, "name", ""),
		getObjectPos(args)
	)
{
	hidden = getBoolOrDefault(args, "hidden", false);
}

physics_params::physics_params(SpaceFloat radius, SpaceFloat mass) :
	dimensions(SpaceVect(radius,0.0)),
	mass(mass)
{}

physics_params::physics_params(SpaceVect dimensions, SpaceFloat mass) :
	dimensions(dimensions),
	mass(mass)
{}

physics_params::physics_params(const ValueMap& args, SpaceFloat mass)
{
	dimensions = getObjectDimensions(args);
	mass = getFloatOrDefault(args, "mass", mass);
}
