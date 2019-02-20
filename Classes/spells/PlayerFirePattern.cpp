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

FlandreWhirlShotPattern::FlandreWhirlShotPattern(Agent *const agent) :
	RadiusPattern(agent, 1.0f, 8),
	FirePattern(agent)
{
}

const float FlandreWideAnglePattern::cooldown = 0.25f;

FlandreWideAnglePattern::FlandreWideAnglePattern(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 4.0, 3),
	FirePattern(agent)
{}

RumiaFastOrbPattern::RumiaFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}

const float RumiaParallelPattern::cooldown = 1.0f / 6.0f;

RumiaParallelPattern::RumiaParallelPattern(Agent *const agent) :
	MultiBulletParallelPattern(agent, cooldown, 1.0, 3),
	FirePattern(agent)
{}

CirnoLargeIceBulletPattern::CirnoLargeIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}

CirnoSmallIceBulletPattern::CirnoSmallIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{}
