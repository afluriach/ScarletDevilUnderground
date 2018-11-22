//
//  Tewi.cpp
//  Koumachika
//
//  Created by Toni on 11/21/18.
//
//

#include "Prefix.h"

#include "Tewi.hpp"

const int Tewi::maxHP = 5;

void Tewi::initStateMachine(ai::StateMachine& sm) {
	sm.addThread(make_shared<ai::EvadePlayerProjectiles>());
}
