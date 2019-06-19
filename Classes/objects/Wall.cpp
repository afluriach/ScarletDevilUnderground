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
#include "physics_context.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	RectangleBody(args)
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions) :
	GObject(make_shared<object_params>(space, id, "", center, 0.0)),
	RectangleBody(dimensions)
{}

PhysicsLayers Wall::getLayers() const{
    return PhysicsLayers::all;
}

BreakableWall::BreakableWall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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
	cpShapeSetSensor(bodyShape, true);
	space->eraseTile(getPos(), "wall_tiles");
	hidden = true;
}
