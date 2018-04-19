//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "GSpace.hpp"
#include "object_ref.hpp"
#include "Sakuya.hpp"
#include "scenes.h"

const int Sakuya::maxHP = 30;

shared_ptr<ai::State> Sakuya::getStartState()
{
    return make_shared<ai::SakuyaBase>();
}        


float Sakuya::getMaxSpeed() const
{
    return 3.0f;
}
