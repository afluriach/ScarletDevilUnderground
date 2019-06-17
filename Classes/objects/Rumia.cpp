//
//  Rumia.cpp
//  Koumachika
//
//  Created by Toni on 2/28/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"
#include "Rumia.hpp"
#include "RumiaSpells.hpp"
#include "value_map.hpp"

Rumia::Rumia(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(collectible_id::nil)
{
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

const AttributeMap Rumia1::baseAttributes = {
	{ Attribute::maxHP, 250.0f },
	{ Attribute::maxMP, 50.0f },
	{ Attribute::agility, 2.0f },
};
const string Rumia1::properName = "Rumia I";

Rumia1::Rumia1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Rumia)
{
	firePattern = make_shared<RumiaBurstPattern>(this);
}

void Rumia1::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (fsm.getThreadCount() > 0) return;
			space->createDialog("dialogs/rumia1", false);
			fsm.addThread(make_shared<RumiaMain1>(&fsm, target));
		}
	);
}

void Rumia1::onZeroHP()
{
	space->createDialog("dialogs/rumia2", false);
	Agent::onZeroHP();
}

const AttributeMap Rumia2::baseAttributes = {
	{ Attribute::maxHP, 500.0f },
	{ Attribute::maxMP, 50.0f },
	{ Attribute::agility, 2.5f },
};
const string Rumia2::properName = "Rumia II";

Rumia2::Rumia2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Rumia)
{
	firePattern = make_shared<RumiaBurstPattern2>(this);
}

void Rumia2::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (fsm.getThreadCount() > 0) return;
			space->createDialog("dialogs/rumia3", false);
			fsm.addThread(make_shared<RumiaDSD2>(&fsm));
			fsm.addThread(make_shared<ai::FireAtTarget>(&fsm, target));
			fsm.addThread(make_shared<ai::Flank>(&fsm, target, 3.0, 1.0));
		}
	);
}

void Rumia2::onZeroHP()
{
	Agent::onZeroHP();
}

const SpaceFloat RumiaMain1::dsdDistMargin = 5.0;
const SpaceFloat RumiaMain1::dsdLength = 5.0;
const SpaceFloat RumiaMain1::dsdCooldown = 15.0;
const float RumiaMain1::dsdCost = 5.0f;

RumiaMain1::RumiaMain1(ai::StateMachine* fsm, gobject_ref target) :
	ai::Function(fsm),
	target(target)
{
}

void RumiaMain1::onEnter()
{
	fsm->addThread(make_shared<ai::Flank>(fsm, target, 3.0, 1.0));
	fsm->addThread(make_shared<ai::FireAtTarget>(fsm, target));
}

void RumiaMain1::onReturn()
{
	fsm->addThread(make_shared<ai::FireAtTarget>(fsm, target));
}

ai::update_return RumiaMain1::update()
{
	timerDecrement(dsdTimer);

	auto& as = *agent->getAttributeSystem();
	bool canCast = dsdTimer <= 0.0 && as[Attribute::mp] >= dsdCost;
	bool willCast =
		ai::distanceToTarget(agent->getPos(), target.get()->getPos()) < dsdDistMargin &&
		as.getMagicRatio() > as.getHealthRatio()
	;

	if (canCast && willCast) {
		dsdTimer = dsdCooldown;
		fsm->removeThread("FireAtTarget");
		return_push( fsm->make<ai::Cast>(make_spell_generator<DarknessSignDemarcation>(), dsdLength) );
	}
	else {
		return_steady();
	}
}

void RumiaMain1::onExit()
{
	fsm->removeThread("Flank");
	fsm->removeThread("FireAtTarget");
}

const vector<double_pair> RumiaDSD2::demarcationSizeIntervals = {
	make_pair(9.0, float_pi * 8.0 / 3.0),
	make_pair(12.0, float_pi * 2.0 / 3.0),
	make_pair(10.0, float_pi * 4.0 / 3.0),
};

void RumiaDSD2::onEnter()
{
	agent->cast(make_shared<DarknessSignDemarcation2>(
		agent,
		demarcationSizeIntervals.at(0).second
	));
}

ai::update_return RumiaDSD2::update()
{
	timerIncrement(timer);

	if (timer >= demarcationSizeIntervals.at(intervalIdx).first) {
		++intervalIdx;
		intervalIdx %= demarcationSizeIntervals.size();
		timer = 0.0;

		agent->cast(make_shared<DarknessSignDemarcation2>(
			agent,
			demarcationSizeIntervals.at(intervalIdx).second
		));
	}
	
	return_steady();
}

void RumiaDSD2::onExit()
{
	agent->stopSpell();
}
