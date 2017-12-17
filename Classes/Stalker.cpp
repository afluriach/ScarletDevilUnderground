//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"
#include "Stalker.hpp"
#include "Player.hpp"
#include "AI.hpp"

const int Stalker::maxHP = 12;

void Stalker::onDetect(GObject* other)
{
    Player* p = dynamic_cast<Player*>(other);

    if(!p){
        log("%s detected non-player object %s", getName().c_str(), other->getName().c_str());
    }
    
    target = p;
}

void Stalker::onEndDetect(GObject* other)
{
    target = nullptr;
}

void Stalker::update()
{
    if(target != nullptr){
        ai::seek(*this, *target, getMaxSpeed(), getMaxAcceleration());
        setDirection(toDirection(ai::directionToTarget(*this, *target)));
    }
    else
        ai::applyDesiredVelocity(*this, SpaceVect(0,0), getMaxAcceleration());
}
