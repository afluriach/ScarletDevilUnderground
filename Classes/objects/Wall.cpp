//
//  Wall.cpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "object_params.hpp"
#include "physics_context.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

GType Wall::getWallType(bool breakable)
{
	return breakable ? enum_bitwise_or(GType, wall, canDamage) : GType::wall;
}

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Wall(space, id, getObjectPos(args), getObjectDimensions(args), getBoolOrDefault(args, "breakable", false))
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, bool breakable) :
	GObject(
		PosAngleParams(center, 0.0),
		physics_params(getWallType(breakable), PhysicsLayers::all, dimensions, -1.0)
	),
	breakable(breakable)
{}

bool Wall::hit(DamageInfo damage)
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
