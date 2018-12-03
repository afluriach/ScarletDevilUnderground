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
	if (cooldownFramesRemaining <= 0 && powerCost() <= agent->getPower())
	{
		cooldownFramesRemaining = getCooldownFrames();
		agent->consumePower(powerCost());
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

	GObject* bullet = spawn(agent->getAngle(), pos);

	if (bullet)
		app->space->addObject(bullet);
}

void SingleBulletFixedIntervalPattern::update()
{
	cooldownFramesRemaining = max(cooldownFramesRemaining - 1, 0);
}

bool SingleBulletFixedIntervalPattern::isInCooldown()
{
	return cooldownFramesRemaining > 0;
}

GObject* FlandreBigOrbPattern::spawn(float angle, SpaceVect pos)
{
	return new FlandreBigOrb1(angle, pos);
}

GObject* FlandreFastOrbPattern::spawn(float angle, SpaceVect pos)
{
	return new FlandreFastOrb1(angle, pos);
}

GObject* CirnoLargeIceBulletPattern::spawn(float angle, SpaceVect pos)
{
	return new CirnoLargeIceBullet(angle, pos);
}

GObject* IceFairyBulletPattern::spawn(float angle, SpaceVect pos)
{
	return new IceFairyBullet(angle, pos);
}

