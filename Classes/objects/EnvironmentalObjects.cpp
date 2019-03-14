//
//  EnvironmentalObjects.cpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "EnvironmentalObjects.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "macros.h"
#include "value_map.hpp"

Headstone::Headstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args),
RectangleBody(args)
{
	setInitialDirectionOrDefault(args, Direction::up);
}

GhostHeadstone::GhostHeadstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Headstone),
	RegisterInit<GhostHeadstone>(this)
{
}

void GhostHeadstone::init()
{
	space->runSpriteAction(spriteID, indefiniteFlickerAction(0.333f, 48, 96));
}

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
}

bool Mushroom::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int objectID = getIntOrDefault(args, "id", -1);
	return objectID != -1 && !space->getState()->isMushroomAcquired(objectID);
}

Mushroom::Mushroom(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
	objectID = getIntOrDefault(args, "id", -1);
}

void Mushroom::interact()
{
	space->getState()->registerMushroomAcquired(objectID);
	++space->getState()->mushroomCount;
	space->removeObject(this);
}
