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

GObject::GeneratorType FlandreBigOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreBigOrb1>(angle, pos);
}

GObject::GeneratorType FlandreFastOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreFastOrb1>(angle, pos);
}

const boost::rational<int> FlandreWideAnglePattern::primaryCooldown = boost::rational<int>(1,6);
const boost::rational<int> FlandreWideAnglePattern::sideCooldown = boost::rational<int>(1,4);

bool FlandreWideAnglePattern::fireIfPossible()
{
	bool hasFired = false;

	SpaceVect pos = agent->getPos();
	SpaceVect primary = SpaceVect::ray(1.0, agent->getAngle());
	SpaceVect side1 = SpaceVect::ray(1.0, agent->getAngle() - float_pi / 4.0);
	SpaceVect side2 = SpaceVect::ray(1.0, agent->getAngle() + float_pi / 4.0);

	if (crntPrimaryCooldown == 0) {
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent->getAngle(), pos + primary));
		crntPrimaryCooldown = primaryCooldown;
		hasFired = true;
	}
	if (crntSideCooldown == 0) {
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent->getAngle() - float_pi / 4.0, pos + side1));
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent->getAngle() + float_pi / 4.0, pos + side2));
		crntSideCooldown = sideCooldown;
		hasFired = true;
	}

	return hasFired;
}

bool FlandreWideAnglePattern::isInCooldown()
{
	return crntPrimaryCooldown != 0 || crntSideCooldown != 0;
}

void FlandreWideAnglePattern::update()
{
	timerDecrement(crntPrimaryCooldown);
	timerDecrement(crntSideCooldown);
}

const boost::rational<int> RumiaParallelPattern::cooldown = boost::rational<int>(1, 6);

bool RumiaParallelPattern::fireIfPossible()
{
	bool hasFired = false;

	SpaceVect pos = agent->getPos();
	SpaceVect facing = SpaceVect::ray(1.0, agent->getAngle());
	SpaceVect perp = SpaceVect::ray(1.0, agent->getAngle() + float_pi / 2.0);

	if (crntCooldown == 0) {
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent->getAngle(), pos + facing));
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent->getAngle(), pos + facing + perp));
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent->getAngle(), pos + facing - perp));

		crntCooldown = cooldown;
		hasFired = true;
	}

	return hasFired;
}

bool RumiaParallelPattern::isInCooldown()
{
	return crntCooldown != 0;
}

void RumiaParallelPattern::update()
{
	timerDecrement(crntCooldown);
}

GObject::GeneratorType RumiaFastOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<RumiaFastOrb1>(angle, pos);
}

GObject::GeneratorType CirnoLargeIceBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoLargeIceBullet>(angle, pos);
}

GObject::GeneratorType CirnoSmallIceBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoSmallIceBullet>(angle, pos);
}

GObject::GeneratorType IceFairyBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<IceFairyBullet>(angle, pos);
}

list<GObject::GeneratorType> Fairy1ABulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	list<GObject::GeneratorType> result;

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle - float_pi / 6.0,
		pos + SpaceVect::ray(getLaunchDistance(), angle - float_pi / 6.0)
	));

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle,
		pos + SpaceVect::ray(getLaunchDistance(), angle)
	));

	result.push_back(GObject::make_object_factory<Fairy1Bullet>(
		angle + float_pi / 6.0,
		pos + SpaceVect::ray(getLaunchDistance(), angle + float_pi / 6.0)
	));

	return result;
}
