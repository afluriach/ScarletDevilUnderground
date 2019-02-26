//
//  Marisa.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "GSpace.hpp"
#include "Marisa.hpp"
#include "util.h"

MarisaNPC::MarisaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

const AttributeMap Marisa::baseAttributes = {
	{Attribute::maxHP, 100.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 6.0f}
};

Marisa::Marisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

CollectMarisa::CollectMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	MapObjForwarding(Marisa)
{}

void CollectMarisa::initStateMachine(ai::StateMachine& sm)
{
	gobject_ref player = space->getObjectRef("player");

	sm.addThread(make_shared<MarisaCollectMain>());
}

void MarisaCollectMain::onEnter(ai::StateMachine& sm)
{
}

void MarisaCollectMain::update(ai::StateMachine& sm)
{
	GObject* player = sm.agent->space->getObject("player");

	if (!player)
		return;

	sm.push(ai::FollowPath::pathToTarget(sm.agent->space, sm.agent, player));
}

ForestMarisa::ForestMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
MapObjForwarding(Agent),
MapObjForwarding(Marisa)
{}

void ForestMarisa::initStateMachine(ai::StateMachine& sm)
{
	sm.addThread(make_shared<MarisaForestMain>());
}

void MarisaForestMain::onEnter(ai::StateMachine& sm)
{
	gobject_ref player = sm.agent->space->getObjectRef("player");
	sm.addThread(make_shared<ai::AimAtTarget>(player));
}

void MarisaForestMain::update(ai::StateMachine& sm)
{
	sm.push(make_shared<ai::Cast>(make_spell_generator<StarlightTyphoon>()));
}