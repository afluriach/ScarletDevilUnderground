//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "MagicEffect.hpp"
#include "value_map.hpp"

const vector<string> StarBullet::colors = boost::assign::list_of
    ("blue")
    ("green")
    ("grey")
    ("indigo")
    ("purple")
    ("red")
    ("yellow")
;

IllusionDialDagger::IllusionDialDagger(const SpaceVect& pos, float angular_velocity) :
GObject("IllusionDialDagger", pos, true)
{
    setInitialAngularVelocity(angular_velocity);
}

float IllusionDialDagger::targetViewAngle()
{
    GObject* target = app->space->getObject("player");

    if(target)
        return ai::viewAngleToTarget(*this,*target);
    else
        return numeric_limits<float>::infinity();
}

void IllusionDialDagger::launch()
{
    GObject* target = app->space->getObject("player");

    if(target){
        setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
        setAngularVel(0.0f);
    }
    else
        debug_log("player missing");
}

FlandreBigOrb1::FlandreBigOrb1(float angle, const SpaceVect& pos) :
	GObject("bigOrb1", pos, angle, true)
{}

FlandreFastOrb1::FlandreFastOrb1(float angle, const SpaceVect& pos) :
	GObject("fastOrb1", pos, angle, true)
{}

CirnoLargeIceBullet::CirnoLargeIceBullet(float angle, const SpaceVect& pos) :
	GObject("bigIce1", pos, angle, true)
{}

shared_ptr<MagicEffect> CirnoLargeIceBullet::getMagicEffect(gobject_ref target)
{
	return make_shared<FrostStatusEffect>(target, 0.5f);
}

