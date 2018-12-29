//
//  Slime.cpp
//  Koumachika
//
//  Created by Toni on 12/24/18.
//
//

#include "Prefix.h"

#include "Slime.hpp"

Slime1::Slime1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1)
{
}

const AttributeMap Slime1::baseAttributes = {
	{ Attribute::maxHP, 15.0f },
	{ Attribute::speed, 3.3f },
	{ Attribute::acceleration, 3.0f }
};

void Slime1::initStateMachine(ai::StateMachine& sm)
{
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::Seek>(target);
		}
	);
	fsm.addThread(detectMain);
}

AttributeMap Slime1::touchEffect() {
	return {
		{ Attribute::hp, -1.0f },
		{ Attribute::slimeDamage, 5.0f }
	};
}

Slime2::Slime2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1)
{
}

const AttributeMap Slime2::baseAttributes = {
	{ Attribute::maxHP, 50.0f },
	{ Attribute::speed, 2.4f },
	{ Attribute::acceleration, 3.0f }
};

void Slime2::initStateMachine(ai::StateMachine& sm)
{
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::Seek>(target);
		}
	);
	fsm.addThread(detectMain);
}

AttributeMap Slime2::touchEffect() {
	return {
		{ Attribute::hp, -1.0f },
		{ Attribute::slimeDamage, 25.0f }
	};
}
