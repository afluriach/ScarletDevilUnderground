//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "scenes.h"
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

CircleBullet::CircleBullet(const ValueMap& args) :
radius(getFloatOrDefault(args, "radius", 0.3f))
{}

PlayerBaseBullet::PlayerBaseBullet(float angle, const SpaceVect& pos) :
GObject("playerBaseBullet", pos)
{
    setInitialVelocity(SpaceVect::ray(getMaxSpeed(), angle));
}

const float IllusionDialDagger::speed = 3.0f;
const SpaceVect IllusionDialDagger::dimensions = SpaceVect(0.25f,1.0f);


IllusionDialDagger::IllusionDialDagger(const SpaceVect& pos, float angular_velocity) :
GObject("IllusionDialDagger", pos),
RectangleBullet(dimensions)
{
    setInitialAngularVelocity(angular_velocity);
}

float IllusionDialDagger::targetViewAngle()
{
    GObject* target = GScene::getSpace()->getObject("player");

    if(target)
        return ai::viewAngleToTarget(*this,*target);
    else
        return numeric_limits<float>::infinity();
}

void IllusionDialDagger::launch()
{
    GObject* target = GScene::getSpace()->getObject("player");

    if(target){
        setVel(ai::directionToTarget(*this, *target)*speed);
        setAngularVel(0.0f);
    }
    else
        debug_log("player missing");
}
