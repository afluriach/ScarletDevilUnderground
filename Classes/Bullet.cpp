//
//  Bullet.cpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"
#include "Bullet.hpp"

void PlayerBaseBullet::init()
{
    body->setVel(SpaceVect::ray(speed,angle));
}

void PatchouliFireBullet::init()
{
    body->setVel(SpaceVect::ray(speed,angle));
}