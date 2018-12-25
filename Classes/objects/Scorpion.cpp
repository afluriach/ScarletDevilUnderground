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
	MapObjForwarding(Agent)
{
}

const AttributeMap Scorpion1::baseAttributes = {
	{ Attribute::maxHP, 15.0f },
	{ Attribute::speed, 4.5f },
	{ Attribute::acceleration, 6.0f }
};

void Scorpion1::initStateMachine(ai::StateMachine& sm)
{
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::Seek>(target);
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
