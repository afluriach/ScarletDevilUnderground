//
//  Scorpion.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Scorpion.hpp"

Scorpion1::Scorpion1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
}

const string Scorpion1::baseAttributes = "scorpion1";
const string Scorpion1::properName = "Scorpion I";

void Scorpion1::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Scurry>(
				&fsm,
				target,
				3.0,
				-1.0
			));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Scurry");
		}
	);

}

DamageInfo Scorpion1::touchEffect() const {
	return DamageInfo{ 5.0f, Attribute::poisonDamage, DamageType::touch };
}

Scorpion2::Scorpion2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
}

const string Scorpion2::baseAttributes = "scorpion2";
const string Scorpion2::properName = "Scorpion II";

void Scorpion2::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Flank>(&fsm, target, 1.0, 1.0));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Flank");
		}
	);

}

DamageInfo Scorpion2::touchEffect() const {
	return DamageInfo{ 15.0f, Attribute::poisonDamage, DamageType::touch };
}
