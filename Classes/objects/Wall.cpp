//
//  Wall.cpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#include "Prefix.h"

#include "enum.h"
#include "GSpace.hpp"
#include "object_params.hpp"
#include "physics_context.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(),MapRectPhys(-1.0))
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions) :
	GObject(
		PosAngleParams(center, 0.0),
		physics_params(dimensions, -1.0)
	)
{}

PhysicsLayers Wall::getLayers() const{
    return PhysicsLayers::all;
}

BreakableWall::BreakableWall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(Wall)
{}

void BreakableWall::hit()
{
	applyBreak();

	BreakableWall* adj = dynamic_cast<BreakableWall*>(space->physicsContext->queryAdjacentTiles(
		getPos(),
		GType::wall,
		PhysicsLayers::all,
		typeid(BreakableWall)
	));

	if (adj && !adj->hidden) {
		adj->applyBreak();
	}
}

void BreakableWall::applyBreak()
{
	setBodySensor(true);
	space->eraseTile(getPos(), "wall_tiles");
	hidden = true;
}
