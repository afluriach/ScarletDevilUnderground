//
//  EnemyFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"

IceFairyBulletPattern::IceFairyBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	EnemyBulletImplPattern(app::getBullet("iceFairyBullet"))
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
	EnemyBulletImplPattern(app::getBullet("fairy1Bullet"))
{
}

GreenFairyBulletPattern::GreenFairyBulletPattern(
	Agent *const agent,
	float fireInterval,
	int bulletCount
) :
	RadiusPattern(agent, fireInterval, bulletCount),
	FirePattern(agent),
	EnemyBulletImplPattern(app::getBullet("greenFairyBullet"))
{
}

BlueFairyFirePattern::BlueFairyFirePattern(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 1.5, 0.5, 3),
	EnemyBulletImplPattern(app::getBullet("blueFairyBullet"))
{
}

RumiaBurstPattern::RumiaBurstPattern(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 1.5, 0.25, 3),
	EnemyBulletImplPattern(app::getBullet("rumiaBullet"))
{}

RumiaBurstPattern2::RumiaBurstPattern2(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 0.9, 0.33, 6),
	EnemyBulletImplPattern(app::getBullet("rumiaBullet2"))
{}

ReimuWavePattern::ReimuWavePattern(Agent *const agent) :
	FirePattern(agent)
{}

bool ReimuWavePattern::fire()
{
	SpaceVect pos = agent->getPos();
	SpaceFloat angle = agent->getAngle();
	auto params = Bullet::makeParams(pos, angle);

	gobject_ref b1 = agent->bulletCheckSpawn<ReimuBullet1>(params);
	gobject_ref b2 = agent->bulletCheckSpawn<ReimuBullet1>(params);

	agent->makeInitMessage(&GObject::setParametricMove, b1, ReimuBullet1::getParametricFunction(
		pos,
		angle,
		0.0
	));

	agent->makeInitMessage(&GObject::setParametricMove, b2, ReimuBullet1::getParametricFunction(
		pos,
		angle,
		0.5
	));

	return true;
}
