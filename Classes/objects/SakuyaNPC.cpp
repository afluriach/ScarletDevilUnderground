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

void SakuyaNPC::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::SakuyaNPC1>());
}
