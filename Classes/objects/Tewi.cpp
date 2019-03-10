//
//  Tewi.cpp
//  Koumachika
//
//  Created by Toni on 11/21/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "Tewi.hpp"

const AttributeMap Tewi::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::speed, 6.0f},
	{Attribute::acceleration, 36.0f}
};

void Tewi::initStateMachine(ai::StateMachine& sm) {
	sm.addThread(make_shared<ai::EvadePlayerProjectiles>());
}
