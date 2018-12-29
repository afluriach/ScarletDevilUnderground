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
RectangleMapBody(args),
spawn_args(getSpawnArgs(args))
{
}

PhysicsLayers Spawner::getLayers() const{
    return PhysicsLayers::ground;
}

void Spawner::onContact(GObject* obj)
{
	contacts.insert(obj);
}

void Spawner::onEndContact(GObject* obj)
{
	contacts.erase(obj);
}

bool Spawner::isObstructed() const {
	return !contacts.empty() || lastSpawnFrame == space->getFrame();
}

void Spawner::activate()
{
	if (!isObstructed())
	{
		lastSpawnFrame = space->getFrame();
		space->createObject(spawn_args);
	}
}