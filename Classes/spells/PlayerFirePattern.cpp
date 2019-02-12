//
//  PlayerFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "GSpace.hpp"
#include "PlayerBullet.hpp"
#include "PlayerFirePattern.hpp"

FlandreBigOrbPattern::FlandreBigOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{
}

FlandreFastOrbPattern::FlandreFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{
}

const boost::rational<int> FlandreWideAnglePattern::cooldown = boost::rational<int>(1,4);

FlandreWideAnglePattern::FlandreWideAnglePattern(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 4.0, 3),
	FirePattern(agent)
{}

RumiaFastOrbPattern::RumiaFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}

const boost::rational<int> RumiaParallelPattern::cooldown = boost::rational<int>(1, 6);

RumiaParallelPattern::RumiaParallelPattern(Agent *const agent) :
	FirePattern(agent)
{}

bool RumiaParallelPattern::fire()
{
	if (isInCooldown())
		return false;

	SpaceVect pos = agent->getPos();
	SpaceVect facing = SpaceVect::ray(1.0, agent->getAngle());
	SpaceVect perp = SpaceVect::ray(1.0, agent->getAngle() + float_pi / 2.0);

	agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing));
	agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing + perp));
	agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing - perp));

	return true;
}

CirnoLargeIceBulletPattern::CirnoLargeIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}

CirnoSmallIceBulletPattern::CirnoSmallIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}
