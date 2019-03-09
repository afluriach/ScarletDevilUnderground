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
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::flandreBigOrb1)
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

const float FlandreWideAnglePattern1::cooldown = 0.25f;

FlandreWideAnglePattern1::FlandreWideAnglePattern1(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 8.0, 3),
	FirePattern(agent)
{}

const float FlandreWideAnglePattern2::cooldown = 0.25f;

FlandreWideAnglePattern2::FlandreWideAnglePattern2(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 4.0, 5),
	FirePattern(agent)
{}

RumiaFastOrbPattern::RumiaFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::rumiaFastOrb1)
{}

const float RumiaParallelPattern::cooldown = 1.0f / 6.0f;

RumiaParallelPattern::RumiaParallelPattern(Agent *const agent) :
	MultiBulletParallelPattern(agent, cooldown, 1.0, 3),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::rumiaFastOrb1)
{}

CirnoLargeIceBulletPattern::CirnoLargeIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::cirnoLargeIceBullet)
{}

CirnoSmallIceBulletPattern::CirnoSmallIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::cirnoSmallIceBullet)
{}
