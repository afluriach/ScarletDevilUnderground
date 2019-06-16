//
//  Slime.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Slime.hpp"

Slime1::Slime1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
}

const AttributeMap Slime1::baseAttributes = {
	{ Attribute::maxHP, 15.0f },
	{ Attribute::speed, 3.3f },
	{ Attribute::acceleration, 3.0f }
};

const string Slime1::properName = "Slime I";

void Slime1::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Seek>(&fsm, target, false));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Seek");
		}
	);
}

DamageInfo Slime1::touchEffect() const {
	return DamageInfo{ 5.0f, Attribute::slimeDamage, DamageType::touch };
}

Slime2::Slime2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
}

const AttributeMap Slime2::baseAttributes = {
	{ Attribute::maxHP, 50.0f },
	{ Attribute::speed, 2.4f },
	{ Attribute::acceleration, 3.0f }
};

const string Slime2::properName = "Slime II";

void Slime2::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Seek>(&fsm, target, true));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Seek");
		}
	);
}

DamageInfo Slime2::touchEffect() const {
	return DamageInfo{ 15.0f, Attribute::slimeDamage, DamageType::touch };
}
