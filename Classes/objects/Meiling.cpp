//
//  Meiling.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "Meiling.hpp"

bool Meiling1::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	return !App::crntState->isChamberCompleted("graveyard3");
}

Meiling1::Meiling1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space, id, args,
		"",
		defaultSize,
		-1.0
	)
{}

string Meiling1::getDialog() {
	return "dialogs/meiling1";
}
