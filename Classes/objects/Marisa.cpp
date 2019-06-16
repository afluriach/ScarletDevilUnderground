//
//  Marisa.cpp
//  Koumachika
//
//  Created by Toni on 12/15/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "GSpace.hpp"
#include "Marisa.hpp"
#include "MarisaSpell.hpp"

class GObject;

MarisaNPC::MarisaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

const AttributeMap Marisa::baseAttributes = {
	{Attribute::maxHP, 100.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 6.0f}
};

const string Marisa::properName = "Marisa Kirisame";

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

void CollectMarisa::initStateMachine()
{
	gobject_ref player = space->getObjectRef("player");

	fsm.addThread(make_shared<MarisaCollectMain>(&fsm));
}

void MarisaCollectMain::onEnter()
{
}

shared_ptr<ai::Function> MarisaCollectMain::update()
{
	GObject* player = agent->space->getObject("player");

	return player ? ai::FollowPath::pathToTarget(fsm, player) : nullptr;
}

ForestMarisa::ForestMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
MapObjForwarding(Agent),
MapObjForwarding(Marisa)
{}

void ForestMarisa::initStateMachine()
{
	fsm.addThread(make_shared<MarisaForestMain>(&fsm));
}

void MarisaForestMain::onEnter()
{
	gobject_ref player = fsm->getSpace()->getObjectRef("player");
	fsm->addThread(make_shared<ai::AimAtTarget>(fsm, player));
}

shared_ptr<ai::Function> MarisaForestMain::update()
{
	return fsm->make<ai::Cast>(make_spell_generator<StarlightTyphoon>());
}
