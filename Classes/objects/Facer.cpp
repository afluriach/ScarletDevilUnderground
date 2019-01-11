//
//  Facer.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "Facer.hpp"
#include "macros.h"

const AttributeMap Facer::baseAttributes = {
	{Attribute::speed, 1.0f },
	{Attribute::acceleration, 4.5f}
};

Facer::Facer(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Facer::hit(AttributeMap attributeEffects, shared_ptr<MagicEffect> effect)
{
    rotate(float_pi/2);
}

void Facer::initStateMachine(ai::StateMachine& sm) {
	sm.addThread(make_shared<FacerMain>());
}

void FacerMain::onEnter(ai::StateMachine& sm)
{
	target = sm.agent->space->getObject("player");
}

void FacerMain::update(ai::StateMachine& sm)
{
	if (target.isValid()) {
		if (ai::isFacingTarget(sm.agent, target.get())) {
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
