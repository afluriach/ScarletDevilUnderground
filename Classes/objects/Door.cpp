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
	Door* dest = space->getObjectAs<Door>(destination);

	if (dest)
	{
		GObject* p = space->getObject("player");

		SpaceVect pos = dest->getPos() + SpaceVect::ray(1.0, dirToPhysicsAngle(dest->entryDirection));

		p->setPos(pos);
		p->setDirection(dest->entryDirection);
	}
}
