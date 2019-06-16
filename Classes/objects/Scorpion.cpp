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
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
}

const AttributeMap Scorpion1::baseAttributes = {
	{ Attribute::maxHP, 25.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 6.0f }
};

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
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
}

const AttributeMap Scorpion2::baseAttributes = {
	{ Attribute::maxHP, 50.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 9.0f }
};

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
