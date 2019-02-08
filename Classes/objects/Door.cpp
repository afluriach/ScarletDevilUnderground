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
#include "value_map.hpp"

Door::Door(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{
	locked = getBoolOrDefault(args, "locked", false);
	entryDirection = stringToDirection(getStringOrDefault(args, "dir", "none"));
	destination = getStringOrDefault(args, "dest", "");
}

PhysicsLayers Door::getLayers() const{
	return PhysicsLayers::all;
}

bool Door::canInteract()
{
	Player* p = space->getObjectAs<Player>("player");

	return getDestination() && !sealed && (!locked || p->getKeyCount() > 0);
}

void Door::interact()
{
	Player* p = space->getObjectAs<Player>("player");

	if (!locked && !sealed) {
		p->useDoor(this);
	}
	else if(locked){
		p->useKey();
		locked = false;
	}
}

string Door::interactionIcon()
{
	return locked ? "sprites/key.png" : "sprites/door.png";
}

void Door::activate()
{
	setSealed(true);
}

void Door::deactivate()
{
	setSealed(false);
}

void Door::setSealed(bool b)
{
	sealed = b;

	if (spriteID != 0) {
		space->setSpriteTexture(spriteID, b ? "sprites/door_locked.png" : "sprites/door.png");
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
