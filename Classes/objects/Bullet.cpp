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

IceFairyBullet::IceFairyBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos) :
	GObject(space, id, "IceFairyBullet", pos, angle, true)
{}

LauncherBullet::LauncherBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos) :
	GObject(space, id, "LauncherBullet", pos, angle, true)
{}

AttributeMap IceFairyBullet::getAttributeEffect() const{
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 25 }
	};
}

shared_ptr<MagicEffect> IceFairyBullet::getMagicEffect(gobject_ref target) {
	return nullptr;
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, float angular_velocity) :
GObject(space,id,"IllusionDialDagger", pos, true)
{
    setInitialAngularVelocity(angular_velocity);
}

float IllusionDialDagger::targetViewAngle()
{
    GObject* target = space->getObject("player");

    if(target)
        return ai::viewAngleToTarget(*this,*target);
    else
        return numeric_limits<float>::infinity();
}

void IllusionDialDagger::launch()
{
    GObject* target = space->getObject("player");

    if(target){
        setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
        setAngularVel(0.0f);
    }
    else
        debug_log("player missing");
}

FlandreBigOrb1::FlandreBigOrb1(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos) :
	GObject(space,id,"bigOrb1", pos, angle, true)
{}

FlandreFastOrb1::FlandreFastOrb1(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos) :
	GObject(space,id,"fastOrb1", pos, angle, true)
{}

CirnoLargeIceBullet::CirnoLargeIceBullet(GSpace* space, ObjectIDType id, float angle, const SpaceVect& pos) :
	GObject(space,id,"bigIce1", pos, angle, true)
{}

AttributeMap CirnoLargeIceBullet::getAttributeEffect() const {
	return {
		{Attribute::hp, -1},
		{Attribute::iceDamage, 50}
	};
}

shared_ptr<MagicEffect> CirnoLargeIceBullet::getMagicEffect(gobject_ref target)
{
	return nullptr;
}

