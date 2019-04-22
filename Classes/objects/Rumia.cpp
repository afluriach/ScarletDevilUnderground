//
//  Rumia.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"
#include "Rumia.hpp"
#include "RumiaSpells.hpp"
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

CircleLightArea Rumia::getLightSource() const
{
	return CircleLightArea{
		getPos(),
		4.0,
		Color4F(0.3f,0.3f,0.3f,-1.0f),
		0.5
	};
}

void Rumia::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			if (sm.getThreadCount() > 0) return;

			sm.agent->space->createDialog("dialogs/rumia1", false);

			sm.addThread(make_shared<ai::Flank>(target, 3.0, 1.0));
//			sm.addThread(make_shared<ai::FireAtTarget>(target));

			sm.addThread(make_shared<RumiaDSD2>());
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
			if (sm.getThreadCount() > 0) return;

			sm.agent->space->createDialog("dialogs/rumia3", false);
			sm.addThread(make_shared<ai::FireAtTarget>(target));
		}
	);
}

void Rumia2::onZeroHP()
{
	Agent::onZeroHP();
}

const vector<double_pair> RumiaDSD2::demarcationSizeIntervals = {
	make_pair(9.0, float_pi * 8.0 / 3.0),
	make_pair(12.0, float_pi * 2.0 / 3.0),
	make_pair(10.0, float_pi * 4.0 / 3.0),
};

void RumiaDSD2::onEnter(ai::StateMachine& fsm)
{
	fsm.agent->cast(make_shared<DarknessSignDemarcation2>(
		fsm.agent,
		demarcationSizeIntervals.at(0).second
	));
}

void RumiaDSD2::update(ai::StateMachine& fsm)
{
	timerIncrement(timer);

	if (timer >= demarcationSizeIntervals.at(intervalIdx).first) {
		++intervalIdx;
		intervalIdx %= demarcationSizeIntervals.size();
		timer = 0.0;

		fsm.agent->cast(make_shared<DarknessSignDemarcation2>(
			fsm.agent,
			demarcationSizeIntervals.at(intervalIdx).second
		));
	}
}

void RumiaDSD2::onExit(ai::StateMachine& fsm)
{
	fsm.agent->stopSpell();
}
