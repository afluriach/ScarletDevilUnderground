//
//  Facer.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"
#include "Facer.hpp"
#include "AI.hpp"

void Facer::init()
{
    target = GScene::getSpace()->getObject("player");
}


void Facer::update()
{
    if(ai::isFacingTarget(this, target))
    {
        setVel(SpaceVect::ray( getMaxSpeed(), getAngle() ));
    }
    else
    {
        setVel(SpaceVect(0,0));
    }
}

void Facer::onPlayerBulletHit(Bullet* bullet)
{
    rotate(float_pi/2);
}
