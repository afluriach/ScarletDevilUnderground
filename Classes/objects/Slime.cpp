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

void Slime1::initStateMachine(ai::StateMachine& sm)
{
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Seek>(target, false));
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
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

void Slime2::initStateMachine(ai::StateMachine& sm)
{
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Seek>(target, true));
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);
}

DamageInfo Slime2::touchEffect() const {
	return DamageInfo{ 15.0f, Attribute::slimeDamage, DamageType::touch };
}
