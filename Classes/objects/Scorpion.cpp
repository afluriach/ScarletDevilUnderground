//
//  Scorpion.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "Scorpion.hpp"

Scorpion1::Scorpion1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1)
{
}

const AttributeMap Scorpion1::baseAttributes = {
	{ Attribute::maxHP, 25.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 6.0f }
};

void Scorpion1::initStateMachine(ai::StateMachine& sm)
{
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[sm](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::Scurry>(
				target->space,
				target,
				3.0,
				-1.0
			);
		}
	);
	fsm.addThread(detectMain);
}

AttributeMap Scorpion1::touchEffect() {
	return {
		{Attribute::hp, -1.0f },
		{Attribute::poisonDamage, 5.0f}
	};
}

Scorpion2::Scorpion2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power2)
{
}

const AttributeMap Scorpion2::baseAttributes = {
	{ Attribute::maxHP, 50.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 9.0f }
};

void Scorpion2::initStateMachine(ai::StateMachine& sm)
{
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[sm](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::Flank>(
				target
			);
		}
	);
	fsm.addThread(detectMain);
}

AttributeMap Scorpion2::touchEffect() {
	return {
		{ Attribute::hp, -1.0f },
		{ Attribute::poisonDamage, 33.3f }
	};
}
