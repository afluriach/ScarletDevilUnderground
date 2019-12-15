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
	InventoryObject(space, id, args, physics_params(GType::playerPickup, PhysicsLayers::ground, 0.5, -1.0))
{}

InventoryObject::InventoryObject(GSpace* space, ObjectIDType id, const ValueMap& args, const physics_params& phys) :
	GObject(MapParams(), phys)
{}

InventoryObject::InventoryObject(shared_ptr<object_params> params, const physics_params& phys) :
	GObject(params, phys)
{}

void InventoryObject::init()
{
	GObject::init();
	updateRoomQuery();
}

void InventoryObject::onPlayerContact()
{
	if (canAcquire()) {
		onAcquire();
		space->getState()->itemRegistry.insert(itemName());

		//remove item object
		space->removeObject(this);
	}
}
