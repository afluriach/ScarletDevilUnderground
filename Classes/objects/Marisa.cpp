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
	MapObjParams(),
	MapObjForwarding(Agent)
{}

const string Marisa::baseAttributes = "marisa";
const string Marisa::properName = "Marisa Kirisame";

Marisa::Marisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

CollectMarisa::CollectMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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

ai::update_return MarisaCollectMain::update()
{
	GObject* player = agent->space->getObject("player");

	if (player)
		return_push(ai::FollowPath::pathToTarget(fsm, player));
	else
		return_pop();
}

ForestMarisa::ForestMarisa(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjParams(),
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

ai::update_return MarisaForestMain::update()
{
	return_push( fsm->make<ai::Cast>(make_spell_generator<StarlightTyphoon>()) );
}
