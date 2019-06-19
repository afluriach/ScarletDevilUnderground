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

const string Follower::baseAttributes = "ff";

Follower::Follower(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	Agent(space, id, args),
	Enemy(collectible_id::nil)
{}

bool Follower::hit(DamageInfo damage)
{
    rotate(-0.5f*float_pi);
	return true;
}

void Follower::initStateMachine() {
	fsm.addThread(make_shared<FollowerMain>(&fsm));
}

void FollowerMain::onEnter()
{
	target = agent->space->getObject("player");
}

ai::update_return FollowerMain::update()
{
	if (target.isValid()) {
		if (ai::isFacingTargetsBack(agent, target.get())) {
			agent->setVel(SpaceVect::ray(agent->getMaxSpeed(), agent->getAngle()));
		}
		else {
			agent->setVel(SpaceVect::zero);
		}
	}
	
	return_pop_if_false( target.isValid() );
}
