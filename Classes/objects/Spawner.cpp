//
//  Spawner.cpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#include "Prefix.h"

#include "Spawner.hpp"
#include "value_map.hpp"

const int Spawner::defaultSpawnLimit = -1;

Spawner::Spawner(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(AreaSensor),
spawn_args(args),
spawnLimit(getIntOrDefault(args, "spawn_limit", defaultSpawnLimit))
{
	string spawnType = getSpawnType();
	if (spawnLimit != -1 && spawnType != "") {
		space->increasePotentialSpawnTotal(spawnType, spawnLimit);
	}
}

gobject_ref Spawner::spawn()
{
	if (!canSpawn())
		return nullptr;

	lastSpawnFrame = space->getFrame();
	++spawnCount;
	return space->createObject(spawn_args);
}

string Spawner::getSpawnType() const
{
	string type = spawn_args.at("type").asString();
	return type;
}

int Spawner::getRemainingSpawns() const
{
	if (spawnLimit == -1) return -1;
	else return spawnLimit - spawnCount;
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

bool Spawner::canSpawn() const {
	return !isObstructed() &&
		(spawnLimit == -1 || spawnCount < spawnLimit) && 
		lastSpawnFrame != space->getFrame();
}

void Spawner::activate()
{
	spawn();
}
