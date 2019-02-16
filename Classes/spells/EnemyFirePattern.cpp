//
//  EnemyFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"

IceFairyBulletPattern::IceFairyBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{
}

Fairy1BulletPattern::Fairy1BulletPattern(
	Agent *const agent,
	boost::rational<int> fireInterval,
	SpaceFloat sideAngleSpread,
	int bulletCount
) :
	MultiBulletSpreadPattern(agent, fireInterval, sideAngleSpread, bulletCount),
	FirePattern(agent)
{
}

GreenFairyBulletPattern::GreenFairyBulletPattern(
	Agent *const agent,
	boost::rational<int> fireInterval,
	int bulletCount
) :
	RadiusPattern(agent, fireInterval, bulletCount),
	FirePattern(agent)
{
}
