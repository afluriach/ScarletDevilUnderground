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

RumiaBurstPattern::RumiaBurstPattern(Agent *const agent) :
	FirePattern(agent),
	BurstPattern(agent, 1.5, 0.25, 3),
	EnemyBulletImplPattern(&EnemyBulletImpl::rumiaBullet)
{}
