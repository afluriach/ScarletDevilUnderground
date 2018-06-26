//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "object_ref.hpp"
#include "Sakuya.hpp"
#include "scenes.h"

const int Sakuya::maxHP = 30;

void Sakuya::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::SakuyaMain>());
}

float Sakuya::getMaxSpeed() const
{
    return 3.0f;
}
