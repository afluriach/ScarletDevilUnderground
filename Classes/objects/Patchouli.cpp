//
//  Patchouli.cpp
//  Koumachika
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "Patchouli.hpp"
#include "Spell.hpp"

Patchouli::Patchouli(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

void Patchouli::initStateMachine(ai::StateMachine& sm)
{

}

const AttributeMap PatchouliEnemy::baseAttributes = {
	{ Attribute::maxHP, 300.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
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

		sm.push(make_shared<ai::Cast1>(make_spell_generator<FlameFence>()));
//		sm.push(make_shared<ai::Cast>("FlameFence", ValueMap()));
	}
}
