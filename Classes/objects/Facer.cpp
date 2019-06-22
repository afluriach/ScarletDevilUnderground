//
//  Facer.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "Facer.hpp"
#include "GSpace.hpp"

const string Facer::baseAttributes = "ff";
const string Facer::properName = "Facer";

Facer::Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Facer::initStateMachine() {
	fsm.addThread(make_shared<FacerMain>(&fsm));
}

void FacerMain::onEnter()
{
	target = getSpace()->getObject("player");
}

ai::update_return FacerMain::update()
{
	if (target.isValid()) {
		if (ai::isFacingTarget(agent, target.get())) {
			agent->setVel(SpaceVect::ray(agent->getMaxSpeed(), agent->getAngle()));
		}
		else {
			agent->setVel(SpaceVect::zero);
		}
	}

	return_pop_if_false( target.isValid() );
}

bool FacerMain::onBulletHit(Bullet* b)
{
	agent->rotate(float_pi / 2);
	return true;
}
