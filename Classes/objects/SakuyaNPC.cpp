//
//  SakuyaNPC.cpp
//  Koumachika
//
//  Created by Toni on 12/27/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "SakuyaNPC.hpp"

const AttributeMap SakuyaNPC::baseAttributes = {
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 4.5f}
};

SakuyaNPC::SakuyaNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

void SakuyaNPC::initStateMachine() {
	addThread(make_shared<SakuyaNPC1>(&fsm));
}

void SakuyaNPC1::onEnter()
{

}

ai::update_return SakuyaNPC1::update()
{
	return_push( fsm->make<ai::Wander>(0.25, 0.75, 4.0, 1.0));
}
