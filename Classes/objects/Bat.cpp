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

const AttributeMap Bat::baseAttributes = {
	{Attribute::maxHP, 50.0f },
	{Attribute::agility, 2.0f },
	{Attribute::touchDamage, 5.0f},
};

const string Bat::properName = "Bat";

Bat::Bat(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
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

shared_ptr<ai::Function> BatMain::update()
{
	return target.isValid() ?
		fsm->make<ai::Flank>(target, 2.0, 1.0) :
		nullptr
	;
}
