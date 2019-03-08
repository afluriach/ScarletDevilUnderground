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
	{ Attribute::maxHP, 50.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};

Rumia::Rumia(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{
//	firePattern = make_shared<RumiaBurstPattern>(this);
}

void Rumia::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			if (sm.isThreadRunning("Cast")) return;

			sm.agent->space->createDialog("dialogs/rumia1", false);

			sm.addThread(make_shared<ai::MaintainDistance>(target, 5.0, 1.5));
//			sm.addThread(make_shared<ai::FireAtTarget>(target));

			sm.addThread(make_shared<ai::Cast>(make_spell_generator<NightSignPinwheel>()));
		}
	);
}

void Rumia::onZeroHP()
{
	space->createDialog("dialogs/rumia2", false);
	Agent::onZeroHP();
}

const AttributeMap Rumia2::baseAttributes = {
	{ Attribute::maxHP, 75.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};

Rumia2::Rumia2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{
	firePattern = make_shared<RumiaBurstPattern>(this);
}

void Rumia2::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			if (sm.isThreadRunning("FireAtTarget")) return;

			sm.agent->space->createDialog("dialogs/rumia3", false);
			sm.addThread(make_shared<ai::FireAtTarget>(target));
		}
	);
}

void Rumia2::onZeroHP()
{
	Agent::onZeroHP();
}
