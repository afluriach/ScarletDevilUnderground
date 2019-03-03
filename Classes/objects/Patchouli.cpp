//
//  Patchouli.cpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "GState.hpp"
#include "Patchouli.hpp"
#include "PatchouliSpell.hpp"

Patchouli::Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

string Patchouli::getDialog() {
	if (App::crntState->hasItem("GraveyardBook1") && !App::crntState->isChamberAvailable(ChamberID::graveyard1)) {
		return "dialogs/graveyard_book";
	}
	else if (App::crntState->hasItem("ForestBook1") && !App::crntState->isChamberAvailable(ChamberID::forest1)) {
		return "dialogs/forest_book";
	}
	else {
		return "dialogs/patchouli1";
	}
}

void Patchouli::onDialogEnd()
{
	if (App::crntState->hasItem("GraveyardBook1")) {
		App::crntState->registerChamberAvailable(ChamberID::graveyard0);
	}
	if (App::crntState->hasItem("ForestBook1")) {
		App::crntState->registerChamberAvailable(ChamberID::forest1);
	}
}

void Patchouli::initStateMachine(ai::StateMachine& sm)
{

}

const AttributeMap PatchouliEnemy::baseAttributes = {
	{ Attribute::maxHP, 300.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};

const vector<SpellGeneratorType> PatchouliEnemy::spells = {
	make_spell_generator<FlameFence>(),
	make_spell_generator<Whirlpool1>(),
	make_spell_generator<Whirlpool2>(),
};

PatchouliEnemy::PatchouliEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{}

const int PatchouliMain::castInterval = 50;

void PatchouliEnemy::initStateMachine(ai::StateMachine& sm)
{
	addThread(make_shared<PatchouliMain>());
}

void PatchouliMain::onEnter(ai::StateMachine& sm)
{
	prevHP = sm.getAgent()->getAttribute(Attribute::hp);
}

void PatchouliMain::update(ai::StateMachine& sm)
{
	float crntHP = sm.getAgent()->getAttribute(Attribute::hp);

	if (prevHP - crntHP >= castInterval) {
		prevHP = crntHP;

		sm.push(make_shared<ai::HPCast>(PatchouliEnemy::spells[spellIdx], castInterval));
		
		spellIdx = (spellIdx + 1) % PatchouliEnemy::spells.size();
	}
}
