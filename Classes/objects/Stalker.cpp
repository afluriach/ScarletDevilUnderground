//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"
#include "Stalker.hpp"

const AttributeMap Stalker::baseAttributes = {
	{Attribute::maxHP, 12.0f},
	{Attribute::speed, 1.5f},
	{Attribute::acceleration, 4.5f}
};

void Stalker::initStateMachine(ai::StateMachine& sm)
{
	sm.addThread(
		make_shared<ai::DetectAndSeekPlayer>()
	);
}
