//
//  Items.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"

GraveyardKey::GraveyardKey(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{}

ForestBook1::ForestBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{}

void ForestBook1::onAcquire()
{
	space->createDialog("dialogs/book_acquired", false);
}

GraveyardBook1::GraveyardBook1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
{}

void GraveyardBook1::onAcquire()
{
	space->createDialog("dialogs/book_acquired", false);
}
