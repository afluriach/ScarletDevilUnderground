//
//  SakuyaNPC.cpp
//  Koumachika
//
//  Created by Toni on 12/27/18.
//
//

#include "Prefix.h"

#include "SakuyaNPC.hpp"

const AttributeMap SakuyaNPC::baseAttributes = {
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 4.5f}
};

SakuyaNPC::SakuyaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

void SakuyaNPC::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<SakuyaNPC1>());
}

void SakuyaNPC1::onEnter(ai::StateMachine& sm)
{

}

void SakuyaNPC1::update(ai::StateMachine& sm)
{
	sm.push(make_shared<ai::Wander>(0.25, 0.75, 4.0, 1.0));
}
