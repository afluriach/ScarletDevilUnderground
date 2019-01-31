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

const unsigned int Spawner::defaultSpawnLimit = 1;

Spawner::Spawner(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
MapObjForwarding(AreaSensor),
spawn_args(getSpawnArgs(args)),
spawnLimit(getIntOrDefault(args, "spawn_limit", defaultSpawnLimit))
{
}

type_index Spawner::getSpawnType() const
{
	return GSpace::enemyNameTypeMap.find(spawn_args.at("type").asString())->second;
}

int Spawner::getRemainingSpawns() const
{
	return spawnLimit - spawnCount;
}

int Spawner::getSpawnLimit() const
{
	return spawnLimit;
}

int Spawner::getSpawnCount() const
{
	return spawnCount;
}

bool Spawner::isObstructed() const {
	return AreaSensor::isObstructed() || lastSpawnFrame == space->getFrame();
}

void Spawner::activate()
{
	if (!isObstructed() && spawnCount < spawnLimit)
	{
		lastSpawnFrame = space->getFrame();
		space->createObject(spawn_args);
		++spawnCount;
	}
}