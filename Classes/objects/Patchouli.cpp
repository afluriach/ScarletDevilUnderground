//
//  Patchouli.cpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Patchouli.hpp"
#include "Spell.hpp"

Patchouli::Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space, id, args,
		"",
		defaultSize,
		-1.0
	)
{}

string Patchouli::getDialog() {
	if (App::crntState->hasItem("GraveyardBook1") && !App::crntState->isChamberAvailable("graveyard1")) {
		return "dialogs/graveyard_book";
	}
	else if (App::crntState->hasItem("ForestBook1") && !App::crntState->isChamberAvailable("forest1")) {
		return "dialogs/forest_book";
	}
	else {
		return "dialogs/patchouli1";
	}
}

void Patchouli::onDialogEnd()
{
	if (App::crntState->hasItem("GraveyardBook1")) {
		App::crntState->registerChamberAvailable("graveyard1");
	}
	if (App::crntState->hasItem("ForestBook1")) {
		App::crntState->registerChamberAvailable("forest1");
	}
}
