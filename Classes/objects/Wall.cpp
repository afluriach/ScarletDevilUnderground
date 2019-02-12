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
#include "value_map.hpp"
#include "Wall.hpp"

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space,id,args,true),
	RectangleBody(args)
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions) :
	GObject(space, id, "", center, 0.0),
	RectangleBody(dimensions)
{}

PhysicsLayers Wall::getLayers() const{
    return PhysicsLayers::all;
}

BreakableWall::BreakableWall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Wall)
{}

void BreakableWall::hit()
{
	applyBreak();
}

void BreakableWall::applyBreak()
{
	cpShapeSetSensor(bodyShape, true);
	space->eraseTile(getPos(), "wall_tiles");
	hidden = true;
}
