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

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
}

bool Mushroom::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int objectID = getIntOrDefault(args, "id", -1);
	return objectID != -1 && !App::crntState->isMushroomAcquired(objectID);
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
	App::crntState->registerMushroomAcquired(objectID);
	++App::crntState->mushroomCount;
	space->removeObject(this);
}