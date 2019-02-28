//
//  Rumia.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "EnemyFirePattern.hpp"
#include "Rumia.hpp"
#include "value_map.hpp"

const AttributeMap Rumia::baseAttributes = {
	{ Attribute::maxHP, 100.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};

Rumia::Rumia(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{
	firePattern = make_shared<RumiaBurstPattern>(this);
}

void Rumia::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::MaintainDistance>(target, 5.0, 1.5));
			sm.addThread(make_shared<ai::FireAtTarget>(target));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
			sm.removeThread("FireAtTarget");
		}
	);
}