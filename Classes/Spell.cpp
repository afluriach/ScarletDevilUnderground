//
//  Spell.cpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "Bullet.hpp"
#include "Spell.hpp"

void FireStarburst::runPeriodic()
{
    SpaceVect pos = caster->body->getPos();
    for(int i=0;i<8; ++i)
    {
        float angle = boost::math::float_constants::pi * i / 4;

        SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

        PatchouliFireBullet* bullet = new PatchouliFireBullet(angle, crntPos);
        GScene::getSpace()->addObject(bullet);
    }
}