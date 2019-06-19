//
//  Patchouli.cpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Patchouli.hpp"
#include "PatchouliSpell.hpp"

Patchouli::Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent)
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

void Patchouli::initStateMachine()
{

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
	make_spell_generator<Whirlpool1>(),
	make_spell_generator<Whirlpool2>(),
};

PatchouliEnemy::PatchouliEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{}

const int PatchouliMain::castInterval = 50;

void PatchouliEnemy::initStateMachine()
{
	addThread(make_shared<ai::HPCastSequence>(&fsm, spells, makeIntervalMap(intervals)));
}

void PatchouliMain::onEnter()
{
}

ai::update_return PatchouliMain::update()
{
	return_steady();
}
