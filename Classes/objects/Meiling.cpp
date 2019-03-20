//
//  Meiling.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Meiling.hpp"

bool Meiling1::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	return !space->getState()->isChamberCompleted(ChamberID::graveyard4);
}

Meiling1::Meiling1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

string Meiling1::getDialog() {
	return "dialogs/meiling1";
}
