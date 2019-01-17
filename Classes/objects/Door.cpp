//
//  Door.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "Door.hpp"
#include "enum.h"
#include "GSpace.hpp"
#include "macros.h"
#include "Player.hpp"
#include "scenes.h"
#include "value_map.hpp"

Door::Door(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{
	entryDirection = stringToDirection(getStringOrDefault(args, "dir", "none"));
	destination = getStringOrDefault(args, "dest", "");
}

PhysicsLayers Door::getLayers() const{
	return PhysicsLayers::all;
}

void Door::interact()
{
	Player* p = space->getObjectAs<Player>("player");

	if (p) {
		p->useDoor(this);
	}
}

void Door::setLocked(bool b)
{
	locked = b;

	if (spriteID != 0) {
		space->getScene()->setSpriteTexture(spriteID, b ? "sprites/door_locked.png" : "sprites/door.png");
	}
}

Door* Door::getDestination()
{
	return space->getObjectAs<Door>(destination);
}

SpaceVect Door::getEntryPosition()
{
	return getPos() + SpaceVect::ray(1.0, dirToPhysicsAngle(entryDirection));
}

Direction Door::getEntryDirection()
{
	return entryDirection;
}
