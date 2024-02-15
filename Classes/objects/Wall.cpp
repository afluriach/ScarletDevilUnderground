//
//  Wall.cpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#include "Prefix.h"

#include "physics_context.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

GType Wall::getWallType(local_shared_ptr<wall_properties> props)
{
	return props && props->breakable ? enum_bitwise_or(GType, wall, canDamage) : GType::wall;
}

Wall::Wall(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<wall_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(getWallType(props), PhysicsLayers::all, -1.0),
		props
	),
	props(props)
{}

bool Wall::hit(DamageInfo damage, SpaceVect n)
{
	if (!props || !props->breakable || !damage.isExplosion()) {
		return false;
	}

	applyBreak();

	Wall* adj = dynamic_cast<Wall*>(space->physicsContext->queryAdjacentTiles(
		getPos(),
		GType::wall,
		PhysicsLayers::all,
		typeid(Wall)
	));

	if (adj && !adj->hidden && adj->props && adj->props->breakable) {
		adj->applyBreak();
	}

	return true;
}

void Wall::applyBreak()
{
	setBodySensor(true);
	space->eraseTile(getPos(), "wall_tiles");
	hidden = true;
}
