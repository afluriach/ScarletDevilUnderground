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
	if (App::crntState->hasItem("GraveyardBook1") && !App::crntState->isChamberAvailable("Graveyard1")) {
		return "dialogs/graveyard_book";
	}
	else if (App::crntState->hasItem("ForestBook1") && !App::crntState->isChamberAvailable("Forest1")) {
		return "dialogs/forest_book";
	}
	else {
		return "dialogs/patchouli1";
	}
}

void Patchouli::onDialogEnd()
{
	if (App::crntState->hasItem("GraveyardBook1")) {
		App::crntState->registerChamberAvailable("Graveyard1");
	}
	if (App::crntState->hasItem("ForestBook1")) {
		App::crntState->registerChamberAvailable("Forest1");
	}
}

const string PatchouliEnemy::baseAttributes = "patchouliEnemy";

const vector<float_pair> PatchouliEnemy::intervals = {
	make_pair(200.0f,250.0f),
	make_pair(150.0f, 180.f),
	make_pair(100.0f,150.0f),
	make_pair(0.0f,50.0f),
};

PatchouliEnemy::PatchouliEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		30.0,
		"magic2",
		false
	)
{}
