//
//  Patchouli.cpp
//  FlansBasement
//
//  Created by Toni on 11/27/15.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "Patchouli.hpp"

void fire(const SpaceVect& pos)
{
    for(int i=0;i<8; ++i)
    {
        float angle = boost::math::float_constants::pi * i / 4;

        SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

        PatchouliFireBullet* bullet = new PatchouliFireBullet(angle, crntPos);
        GScene::getSpace()->addObject(bullet);
    }
}

void Patchouli::update()
{
    timeSinceFire += App::secondsPerFrame;
    if(timeSinceFire > 0.5)
    {
        fire(body->getPos());
        timeSinceFire = 0;
    }
}