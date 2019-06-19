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

const string Bat::baseAttributes = "bat";
const string Bat::properName = "Bat";

Bat::Bat(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Bat::initStateMachine() {
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<BatMain>(&fsm));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("BatMain");
		}
	);
}

void BatMain::onEnter()
{
	target = agent->space->getObject("player");
}

ai::update_return BatMain::update()
{
	if (target.isValid())
		return_push(fsm->make<ai::Flank>(target, 2.0, 1.0));
	else
		return_pop();
}
