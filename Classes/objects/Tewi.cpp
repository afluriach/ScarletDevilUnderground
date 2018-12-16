//
//  Tewi.cpp
//  Koumachika
//
//  Created by Toni on 11/21/18.
//
//

#include "Prefix.h"

#include "Tewi.hpp"

const AttributeMap Tewi::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::speed, 2.4f},
	{Attribute::acceleration, 6.0f}
};

void Tewi::initStateMachine(ai::StateMachine& sm) {
	sm.addThread(make_shared<ai::EvadePlayerProjectiles>());
}
