//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "util.h"

const vector<string> StarBullet::colors = boost::assign::list_of
    ("blue")
    ("green")
    ("grey")
    ("indigo")
    ("purple")
    ("red")
    ("yellow")
;

Bullet::Bullet(const ValueMap& args) :
radius(getFloatOrDefault(args, "radius", 0.3f))
{}

PlayerBaseBullet::PlayerBaseBullet(float angle, const SpaceVect& pos) :
GObject("playerBaseBullet", pos)
{
    setInitialVelocity(SpaceVect::ray(getMaxSpeed(), angle));
}
