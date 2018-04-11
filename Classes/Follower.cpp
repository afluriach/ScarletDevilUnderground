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

void Follower::onPlayerBulletHit(Bullet* bullet)
{
    rotate(float_pi/2);
}
