//
//  PlayerFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "PlayerFirePattern.hpp"

const float StarbowBreak::baseCost = 1.0f;
const float StarbowBreak::baseFireInterval = 1.0f / 5.0f;
const array<float, StarbowBreak::anglesCount> StarbowBreak::angleIntervalScales = {
	0.6f,
	0.5f,
	0.75f,
	0.33f,
	0.4f
};

const double StarbowBreak::angleVariation = float_pi * 0.25;
const double StarbowBreak::angleStep = angleVariation / (anglesCount - 1);

const double StarbowBreak::launchDist = 1.0;
const array<double, StarbowBreak::anglesCount> StarbowBreak::radiusScales = {
	0.75,
	0.5,
	2.0 / 3.0,
	0.25,
	1.0 / 3.0
};

StarbowBreak::StarbowBreak(Agent *const agent) :
	FirePattern(agent)
{}

local_shared_ptr<bullet_properties> StarbowBreak::getProps(int angle)
{
	return app::getBullet("starbowBullet" + boost::lexical_cast<string>(angle + 1));
}

bool StarbowBreak::spawnBullet(int angle, bool left)
{
	double _angle = agent->getAngle() + angleStep * angle * (left ? 1.0 : -1.0);
	SpaceVect pos = SpaceVect::ray(launchDist, _angle);
	float cost = baseCost * radiusScales[angle];
	bool fired = false;

	if (agent->get(Attribute::stamina) >= cost) {
		fired = agent->launchBullet(
			getProps(angle),
			pos,
			_angle 
		).isFuture();

		if (fired)
			agent->getAttributeSystem()->modifyAttribute(Attribute::stamina, -cost);
	}

	return fired;
}

bool StarbowBreak::fire()
{
	bool fired = false;

	if (timers[0] <= 0.0f && spawnBullet(0,false))
	{
		timers[0] = baseFireInterval;
		fired = true;
	}

	//angle zero represents forward direction (single bullet)
	//others represent pair of side-angle bullets
	for_irange(i, 1, anglesCount)
	{
		if (timers[i] <= 0.0f) {
			bool legFired = false;
			legFired |= spawnBullet(i, false);
			legFired |= spawnBullet(i, true);

			if (legFired) {
				timers[i] = baseFireInterval;
				fired = true;
			}
		}
	}

	return fired;
}

void StarbowBreak::update()
{
	double attackSpeed = agent->get(Attribute::attackSpeed);
	for_irange(i, 0, anglesCount) {
		timerDecrement(timers[i], angleIntervalScales[i] * attackSpeed);
	}
}

const int Catadioptric::tailCount = 3;
const int Catadioptric::secondaryBulletCount = 6;
const int Catadioptric::tertiaryBulletCount = 12;
const SpaceFloat Catadioptric::secondarySpeedVariation = 2.0;
const SpaceFloat Catadioptric::tertiarySpeedVariation = 1.0;
const SpaceFloat Catadioptric::angleSpread = float_pi / 8.0;
const SpaceFloat Catadioptric::fireInterval = 1.5;

Catadioptric::Catadioptric(Agent *const agent) :
	FirePattern(agent)
{}

bool Catadioptric::spawnTail(SpaceFloat angleOffset)
{
	bool fired = false;
	double _angle = agent->getAngle() + angleOffset;
	SpaceVect pos = SpaceVect::ray(1.0, _angle);

	fired |= agent->launchBullet(
		app::getBullet("catadioptricBullet1"),
		pos,
		_angle
	).isFuture();

	for_irange(i, 0, secondaryBulletCount)
	{
		auto props = app::getBullet("catadioptricBullet2");

		SpaceFloat variation = secondarySpeedVariation * 2.0;
		SpaceFloat step = variation / (secondaryBulletCount - 1);
		SpaceFloat actualAngle = _angle + getSpace()->getRandomFloat(-1.0f, 1.0f)*angleSpread / 4.0;

		auto attr = agent->getBulletAttributes(props);
		attr.speedOffset = -secondarySpeedVariation + step * i;

		agent->_launchBullet(attr, props, pos, actualAngle, 0.0, true);
	}

	for_irange(i, 0, tertiaryBulletCount)
	{
		auto props = app::getBullet("catadioptricBullet3");

		SpaceFloat variation = tertiarySpeedVariation * 2.0;
		SpaceFloat step = variation / (tertiaryBulletCount - 1);
		SpaceFloat actualAngle = _angle + getSpace()->getRandomFloat(-1.0f, 1.0f)*angleSpread / 2.0;

		auto attr = agent->getBulletAttributes(props);
		attr.speedOffset = -tertiarySpeedVariation + step * i;

		agent->_launchBullet(attr, props, pos, actualAngle, 0.0, true);
	}

	return fired;
}

bool Catadioptric::fire()
{
	if (cooldown > 0.0)
		return false;

	bool fired = false;
	SpaceFloat spread = angleSpread * 2.0;
	SpaceFloat angleStep = spread / (tailCount - 1);

	for_irange(i, 0, tailCount)
	{
		fired |= spawnTail( -angleSpread + angleStep*i);
	}

	if (fired)
		cooldown = fireInterval;

	return fired;
}
