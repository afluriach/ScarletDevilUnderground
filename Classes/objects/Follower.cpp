//
//  Follower.cpp
//  Koumachika
//
//  Created by Toni on 3/25/18.
//
//

#include "Prefix.h"

#include "Follower.hpp"
#include "macros.h"

void Follower::onPlayerBulletHit(Bullet* bullet)
{
    rotate(-0.5f*float_pi);
}