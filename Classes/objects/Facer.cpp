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

const AttributeMap Facer::baseAttributes = {
	{Attribute::speed, 1.0f },
	{Attribute::acceleration, 4.5f}
};

const string Facer::properName = "Facer";

Facer::Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

bool Facer::hit(DamageInfo damage)
{
    rotate(float_pi/2);
	return true;
}

void Facer::initStateMachine() {
	fsm.addThread(make_shared<FacerMain>(&fsm));
}

void FacerMain::onEnter()
{
	target = agent->space->getObject("player");
}

void FacerMain::update()
{
	if (target.isValid()) {
		if (ai::isFacingTarget(agent, target.get())) {
			agent->setVel(SpaceVect::ray(agent->getMaxSpeed(), agent->getAngle()));
		}
		else {
			agent->setVel(SpaceVect::zero);
		}
	}
	else {
		pop();
	}
}
