//
//  FirePattern.cpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "Bullet.hpp"
#include "FirePattern.hpp"
#include "GSpace.hpp"
#include "Player.hpp"

bool FirePattern::fireIfPossible()
{
	if (cooldownTimeRemaining <= 0)
	{
		fire();
		cooldownTimeRemaining = getCooldownTime() / agent->getAttribute(Attribute::attackSpeed);

		return true;
	}
	else {
		return false;
	}
}


bool FirePattern::isInCooldown()
{
	return cooldownTimeRemaining > 0;
}

void FirePattern::update()
{
	timerDecrement(cooldownTimeRemaining);
}

bool SingleBulletFixedIntervalPattern::fire()
{
	if (isInCooldown())
		return false;

	SpaceVect pos = agent->getPos();
	pos += SpaceVect::ray(getLaunchDistance(), agent->getAngle());

	agent->space->createObject(spawn(agent->getAngle()));

	return true;
}

RadiusPattern::RadiusPattern(
	Agent *const agent,
	float fireInterval,
	int bulletCount
) :
	FirePattern(agent),
	fireInterval(fireInterval),
	bulletCount(bulletCount)
{
}

bool RadiusPattern::fire()
{
	if (isInCooldown())
		return false;

	SpaceFloat angleStep = float_pi * 2.0 / bulletCount;

	for_irange(i, 0, bulletCount)
	{
		agent->space->createObject(spawn(angleStep * i));
	}

	return true;
}


MultiBulletSpreadPattern::MultiBulletSpreadPattern(
	Agent *const agent,
	float fireInterval,
	SpaceFloat sideAngleSpread,
	int bulletCount
) :
	FirePattern(agent),
	fireInterval(fireInterval),
	sideAngleSpread(sideAngleSpread),
	bulletCount(bulletCount)
{
}

bool MultiBulletSpreadPattern::fire()
{
	if (isInCooldown())
		return false;

	SpaceFloat angle = agent->getAngle();
	SpaceFloat spread = sideAngleSpread * 2.0;
	SpaceFloat angleStep = spread / (bulletCount - 1);

	for_irange(i, 0, bulletCount)
	{
		agent->space->createObject(spawn(angle - sideAngleSpread + angleStep*i));
	}

	return true;
}

MultiBulletParallelPattern::MultiBulletParallelPattern(
	Agent *const agent,
	float fireInterval,
	SpaceFloat bulletSpacing,
	int bulletCount
) :
	FirePattern(agent),
	fireInterval(fireInterval),
	bulletSpacing(bulletSpacing),
	bulletCount(bulletCount)
{
}

bool MultiBulletParallelPattern::fire()
{
	if (isInCooldown())
		return false;

	SpaceFloat width = bulletSpacing * (bulletCount - 1);
	SpaceVect startOffset = SpaceVect::ray(width/2.0, agent->getAngle() - float_pi / 2.0);
	SpaceVect perp = SpaceVect::ray(bulletSpacing, agent->getAngle() + float_pi / 2.0);

	for_irange(i, 0, bulletCount)
	{
		agent->space->createObject(spawn(startOffset + perp*i, agent->getAngle()));
	}

	return true;
}

BurstPattern::BurstPattern(
	Agent *const agent,
	float burstInterval,
	float burstLength,
	int bulletsPerBurst
) :
	FirePattern(agent),
	burstInterval(burstInterval),
	burstLength(burstLength),
	bulletsPerBurst(bulletsPerBurst)
{
}

bool BurstPattern::isInCooldown()
{
	return countdownTimer > 0.0f;
}

void BurstPattern::update()
{
	timerDecrement(countdownTimer);
}


bool BurstPattern::fire()
{
	if (countdownTimer > 0.0f) return false;

	agent->space->createObject(spawn(agent->getAngle()));

	++crntBurstCount;

	if (crntBurstCount >= bulletsPerBurst) {
		countdownTimer = burstInterval - burstLength;
		crntBurstCount = 0;
	}
	else{
		countdownTimer = burstLength / bulletsPerBurst;
	}

	return true;
}

