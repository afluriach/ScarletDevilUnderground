//
//  Follower.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "Follower.hpp"
#include "GSpace.hpp"
#include "macros.h"

const AttributeMap Follower::baseAttributes = {
	{Attribute::speed, 1.0f },
	{Attribute::acceleration, 4.5f}
};

Follower::Follower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	Agent(space, id, args),
	Enemy(collectible_id::nil)
{}

bool Follower::hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect)
{
    rotate(-0.5f*float_pi);
	return true;
}

void Follower::initStateMachine(ai::StateMachine& sm) {
	sm.addThread(make_shared<FollowerMain>());
}

void FollowerMain::onEnter(ai::StateMachine& sm)
{
	target = sm.agent->space->getObject("player");
}

void FollowerMain::update(ai::StateMachine& sm)
{
	if (target.isValid()) {
		if (ai::isFacingTargetsBack(sm.agent, target.get())) {
			sm.agent->setVel(SpaceVect::ray(sm.agent->getMaxSpeed(), sm.agent->getAngle()));
		}
		else {
			sm.agent->setVel(SpaceVect::zero);
		}
	}
	else {
		sm.pop();
	}
}
