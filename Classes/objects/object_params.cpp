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
	dimensions = getObjectDimensions(args);
	hidden = getBoolOrDefault(args, "hidden", false);
}
