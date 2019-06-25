//
//  InventoryObject.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "GState.hpp"
#include "InventoryObject.hpp"

InventoryObject::InventoryObject(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(0.5, -1.0))
{}

void InventoryObject::onPlayerContact()
{
	if (canAcquire()) {
		onAcquire();
		space->getState()->itemRegistry.insert(itemName());

		//remove item object
		space->removeObject(this);
	}
}
