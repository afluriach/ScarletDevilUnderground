//
//  InventoryObject.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "InventoryObject.hpp"

void InventoryObject::interact()
{
    onAcquire();
    GState::crntState.itemRegistry.insert(itemName());
    
    //remove item object
    space->removeObject(this);
}
