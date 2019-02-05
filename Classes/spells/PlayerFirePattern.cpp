//
//  PlayerFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "GSpace.hpp"
#include "PlayerBullet.hpp"
#include "PlayerFirePattern.hpp"

GObject::GeneratorType FlandreBigOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreBigOrb1>(agent, angle, pos);
}

GObject::GeneratorType FlandreFastOrbPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<FlandreFastOrb1>(agent, angle, pos);
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
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent, agent->getAngle(), pos + primary));
		crntPrimaryCooldown = primaryCooldown;
		hasFired = true;
	}
	if (crntSideCooldown == 0) {
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent, agent->getAngle() - float_pi / 4.0, pos + side1));
		agent->space->createObject(GObject::make_object_factory<FlandreFastOrb1>(agent, agent->getAngle() + float_pi / 4.0, pos + side2));
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
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing));
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing + perp));
		agent->space->createObject(GObject::make_object_factory<RumiaFastOrb1>(agent, agent->getAngle(), pos + facing - perp));

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
	return GObject::make_object_factory<RumiaFastOrb1>(agent, angle, pos);
}

GObject::GeneratorType CirnoLargeIceBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoLargeIceBullet>(agent, angle, pos);
}

GObject::GeneratorType CirnoSmallIceBulletPattern::spawn(SpaceFloat angle, SpaceVect pos)
{
	return GObject::make_object_factory<CirnoSmallIceBullet>(agent, angle, pos);
}
