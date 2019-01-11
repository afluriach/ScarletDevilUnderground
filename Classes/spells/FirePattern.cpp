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

bool SingleBulletFixedIntervalPattern::fireIfPossible()
{
	if (cooldownTimeRemaining <= 0)
	{
		cooldownTimeRemaining = getCooldownTime();
		fire();

		return true;
	}
	else {
		return false;
	}
}

void SingleBulletFixedIntervalPattern::fire()
{
	SpaceVect pos = agent->getPos();
	pos += SpaceVect::ray(getLaunchDistance(), agent->getAngle());

	agent->space->createObject(spawn(agent->getAngle(), pos));
}

void SingleBulletFixedIntervalPattern::update()
{
	timerDecrement(cooldownTimeRemaining);
}

bool SingleBulletFixedIntervalPattern::isInCooldown()
{
	return cooldownTimeRemaining > 0;
}

bool MultiBulletFixedIntervalPattern::fireIfPossible()
{
	if (cooldownTimeRemaining <= 0)
	{
		cooldownTimeRemaining = getCooldownTime();
		fire();

		return true;
	}
	else {
		return false;
	}
}

void MultiBulletFixedIntervalPattern::fire()
{
	SpaceVect pos = agent->getPos();

	list<ObjectGeneratorType> spawns = spawn(agent->getAngle(), agent->getPos());

	for (auto gen : spawns) {
		agent->space->createObject(gen);
	}
}

void MultiBulletFixedIntervalPattern::update()
{
	timerDecrement(cooldownTimeRemaining);
}

bool MultiBulletFixedIntervalPattern::isInCooldown()
{
	return cooldownTimeRemaining > 0;
}
