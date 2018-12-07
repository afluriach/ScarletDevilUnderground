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

	app->space->createObject(spawn(agent->getAngle(), pos));
}

void SingleBulletFixedIntervalPattern::update()
{
	cooldownFramesRemaining = max(cooldownFramesRemaining - 1, 0);
}

bool SingleBulletFixedIntervalPattern::isInCooldown()
{
	return cooldownFramesRemaining > 0;
}

GObject::GeneratorType FlandreBigOrbPattern::spawn(float angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreBigOrb1>(angle, pos);
}

GObject::GeneratorType FlandreFastOrbPattern::spawn(float angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreFastOrb1>(angle, pos);
}

GObject::GeneratorType CirnoLargeIceBulletPattern::spawn(float angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoLargeIceBullet>(angle, pos);
}

GObject::GeneratorType IceFairyBulletPattern::spawn(float angle, SpaceVect pos)
{
	return GObject::make_object_factory<IceFairyBullet>(angle, pos);
}

