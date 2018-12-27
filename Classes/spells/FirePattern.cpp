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
	if (cooldownTimeRemaining <= 0 && powerCost() <= agent->getPower())
	{
		cooldownTimeRemaining = getCooldownTime();
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

GObject::GeneratorType FlandreBigOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreBigOrb1>(angle, pos);
}

GObject::GeneratorType FlandreFastOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreFastOrb1>(angle, pos);
}

GObject::GeneratorType RumiaFastOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<RumiaFastOrb1>(angle, pos);
}

GObject::GeneratorType CirnoLargeIceBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoLargeIceBullet>(angle, pos);
}

GObject::GeneratorType IceFairyBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<IceFairyBullet>(angle, pos);
}

