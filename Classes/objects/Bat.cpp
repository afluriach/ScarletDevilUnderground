//
//  Bat.cpp
//  Koumachika
//
//  Created by Toni on 2/20/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Bat.hpp"
#include "GSpace.hpp"
#include "macros.h"

const AttributeMap Bat::baseAttributes = {
	{Attribute::maxHP, 25.0f },
	{Attribute::agility, 2.0f },
};

Bat::Bat(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Bat::initStateMachine(ai::StateMachine& sm) {
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<BatMain>());
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("BatMain");
		}
	);
}

void BatMain::onEnter(ai::StateMachine& sm)
{
	target = sm.agent->space->getObject("player");
}

void BatMain::update(ai::StateMachine& sm)
{
	if (!target.isValid()) sm.pop();

	sm.push(make_shared<ai::Flank>(target, 2.0, 1.0));
}
