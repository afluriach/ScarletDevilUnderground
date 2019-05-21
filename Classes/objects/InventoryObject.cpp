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

void InventoryObject::onPlayerContact()
{
	if (canAcquire()) {
		onAcquire();
		space->getState()->itemRegistry.insert(itemName());

		//remove item object
		space->removeObject(this);
	}
}
