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

GType Wall::getWallType(bool breakable)
{
	return breakable ? enum_bitwise_or(GType, wall, canDamage) : GType::wall;
}

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Wall(space, id, SpaceRect(getObjectPos(args), getObjectDimensions(args)), getBoolOrDefault(args, "breakable", false))
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceRect rect, bool breakable) :
	GObject(
		space,
		id,
		object_params(rect),
		physics_params(getWallType(breakable), PhysicsLayers::all, -1.0),
		nullptr
	),
	breakable(breakable)
{}

bool Wall::hit(DamageInfo damage, SpaceVect n)
{
	if (!breakable || !damage.isExplosion()) {
		return false;
	}

	applyBreak();

	Wall* adj = dynamic_cast<Wall*>(space->physicsContext->queryAdjacentTiles(
		getPos(),
		GType::wall,
		PhysicsLayers::all,
		typeid(Wall)
	));

	if (adj && !adj->hidden && adj->breakable) {
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
