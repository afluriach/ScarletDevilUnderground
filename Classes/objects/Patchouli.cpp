//
//  Patchouli.cpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "GState.hpp"
#include "Patchouli.hpp"
#include "PatchouliSpell.hpp"

Patchouli::Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	NPC(
		space, id, args,
		"",
		defaultSize,
		-1.0
	)
{}

string Patchouli::getDialog() {
	if (space->getState()->hasItem("GraveyardBook1") && !space->getState()->isChamberAvailable(ChamberID::graveyard1)) {
		return "dialogs/graveyard_book";
	}
	else if (space->getState()->hasItem("ForestBook1") && !space->getState()->isChamberAvailable(ChamberID::forest1)) {
		return "dialogs/forest_book";
	}
	else {
		return "dialogs/patchouli1";
	}
}

void Patchouli::onDialogEnd()
{
	if (space->getState()->hasItem("GraveyardBook1")) {
		space->getState()->registerChamberAvailable(ChamberID::graveyard1);
	}
	if (space->getState()->hasItem("ForestBook1")) {
		space->getState()->registerChamberAvailable(ChamberID::forest1);
	}
}

const string PatchouliEnemy::baseAttributes = "patchouliEnemy";

const vector<float_pair> PatchouliEnemy::intervals = {
	make_pair(200.0f,250.0f),
	make_pair(125.0f, 180.f),
	make_pair(100.0f,200.0f),
	make_pair(0.0f,50.0f),
};

const vector<SpellGeneratorType> PatchouliEnemy::spells = {
	make_spell_generator<FireStarburst>(),
	make_spell_generator<FlameFence>(),
	ScriptedSpell::generator("Whirlpool1"),
	ScriptedSpell::generator("Whirlpool2"),
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
