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
	FirePattern(agent),
	EnemyBulletImplPattern(&EnemyBulletImpl::iceFairyBullet)
{
}

Fairy1BulletPattern::Fairy1BulletPattern(
	Agent *const agent,
	float fireInterval,
	SpaceFloat sideAngleSpread,
	int bulletCount
) :
	MultiBulletSpreadPattern(agent, fireInterval, sideAngleSpread, bulletCount),
	FirePattern(agent),
	EnemyBulletImplPattern(&EnemyBulletImpl::fairy1Bullet)
{
}

GreenFairyBulletPattern::GreenFairyBulletPattern(
	Agent *const agent,
	float fireInterval,
	int bulletCount
) :
	RadiusPattern(agent, fireInterval, bulletCount),
	FirePattern(agent),
	EnemyBulletImplPattern(&EnemyBulletImpl::greenFairyBullet)
{
}

BlueFairyFirePattern::BlueFairyFirePattern(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 1.5, 0.5, 3),
	EnemyBulletImplPattern(&EnemyBulletImpl::blueFairyBullet)
{
}

RumiaBurstPattern::RumiaBurstPattern(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 1.5, 0.25, 3),
	EnemyBulletImplPattern(&EnemyBulletImpl::rumiaBullet)
{}

RumiaBurstPattern2::RumiaBurstPattern2(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 0.9, 0.33, 6),
	EnemyBulletImplPattern(&EnemyBulletImpl::rumiaBullet2)
{}

ReimuWavePattern::ReimuWavePattern(Agent *const agent) :
	FirePattern(agent)
{}

bool ReimuWavePattern::fire()
{
	SpaceFloat angle = agent->getAngle();

	agent->bulletCheckSpawn<ReimuBullet1>(
		agent->getPos(),
		angle,
		0
	);
	agent->bulletCheckSpawn<ReimuBullet1>(
		agent->getPos(),
		angle,
		0.5
	);

	return true;
}
