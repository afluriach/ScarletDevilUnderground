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

const int Spawner::defaultSpawnLimit = -1;

Spawner::Spawner(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(AreaSensor),
spawn_args(args),
spawnLimit(getIntOrDefault(args, "spawn_limit", defaultSpawnLimit))
{
	type_index spawnType = getSpawnType();
	if (spawnLimit != defaultSpawnLimit && spawnType != typeid(Spawner)) {
		space->increaseSpawnTotal(spawnType, spawnLimit);
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

type_index Spawner::getSpawnType() const
{
	string type = spawn_args.at("type").asString();
	const object_info* info = getObjectInfo(type);
	if (info) {
		return info->type;
	}
	else {
		log("Spawner::getSpawnType(): unknown enemy type %s.", type.c_str());
		return typeid(Spawner);
	}
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
