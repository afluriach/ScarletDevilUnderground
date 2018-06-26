//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"
#include "Stalker.hpp"

const int Stalker::maxHP = 12;

void Stalker::initStateMachine(ai::StateMachine& sm)
{
	sm.addThread(
		make_shared<ai::DetectAndSeekPlayer>()
	);
}
