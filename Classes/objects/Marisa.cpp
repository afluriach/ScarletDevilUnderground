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

const AttributeMap Marisa::baseAttributes = {
	{Attribute::maxHP, 100.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 6.0f}
};

void Marisa::initStateMachine(ai::StateMachine& sm)
{
	gobject_ref player = space->getObjectRef("player");

	sm.addThread(make_shared<MarisaMain>());
}

void MarisaMain::onEnter(ai::StateMachine& sm)
{
}

void MarisaMain::update(ai::StateMachine& sm)
{
	GObject* player = sm.agent->space->getObject("player");

	if (!player)
		return;

	sm.push(ai::FollowPath::pathToTarget(sm.agent->space, sm.agent, player));
}
