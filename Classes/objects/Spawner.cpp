//
//  Spawner.cpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "Spawner.hpp"
#include "value_map.hpp"

Spawner::Spawner(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
MapObjForwarding(AreaSensor),
spawn_args(getSpawnArgs(args))
{
}

bool Spawner::isObstructed() const {
	return AreaSensor::isObstructed() || lastSpawnFrame == space->getFrame();
}

void Spawner::activate()
{
	if (!isObstructed())
	{
		lastSpawnFrame = space->getFrame();
		space->createObject(spawn_args);
	}
}