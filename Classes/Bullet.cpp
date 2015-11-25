//
//  Bullet.cpp
//  FlansBasement
//
//  Created by Toni on 11/24/15.
//
//

#include "Bullet.hpp"

void PlayerBaseBullet::init()
{
    body->setVel(cp::Vect::ray(speed,angle));
}