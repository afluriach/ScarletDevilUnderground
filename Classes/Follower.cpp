//
//  Follower.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"
#include "Follower.hpp"
#include "AI.hpp"

void Follower::init()
{
    target = GScene::getSpace()->getObject("player");
}


void Follower::update()
{
    if(ai::isFacingTargetsBack(this, target))
    {
        setVel(SpaceVect::ray( getMaxSpeed(), getAngle() ));
    }
    else
    {
        setVel(SpaceVect(0,0));
    }
}

void Follower::onPlayerBulletHit(Bullet* bullet)
{
    rotate(float_pi/2);
}
