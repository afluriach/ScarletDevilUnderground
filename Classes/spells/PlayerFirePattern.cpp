//
//  PlayerFirePattern.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "GSpace.hpp"
#include "PlayerBullet.hpp"
#include "PlayerFirePattern.hpp"

MagicMissile::MagicMissile(Agent *const agent, int level) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("flandreFastOrb1"))
{
}

const float StarbowBreak::baseDamage = 2.0f;
const float StarbowBreak::baseCost = 1.0f;
const float StarbowBreak::baseFireInterval = 1.0f / 5.0f;
const array<float, StarbowBreak::anglesCount> StarbowBreak::angleIntervalScales = {
	0.6f,
	0.5f,
	0.75f,
	0.33f,
	0.4f
};

const double StarbowBreak::baseMass = 1.0;
const double StarbowBreak::baseSpeed = 6.0;

const double StarbowBreak::angleVariation = float_pi * 0.25;
const double StarbowBreak::angleStep = angleVariation / (anglesCount - 1);

const double StarbowBreak::launchDist = 1.0;
const double StarbowBreak::baseRadius = 0.15;
const array<double, StarbowBreak::anglesCount> StarbowBreak::radiusScales = {
	0.75,
	0.5,
	2.0 / 3.0,
	0.25,
	1.0 / 3.0
};

StarbowBreak::StarbowBreak(Agent *const agent, int level) :
	FirePattern(agent)
{}

bullet_properties StarbowBreak::generateProps(int angle)
{
	//specify size multiplier for angle steps
	//damage and mass should be proportional to size
	//speed should be inversely proportional to size
	//angle colors
	double sizeScale = radiusScales[angle];

	return bullet_properties{
		baseMass *sizeScale*sizeScale,
		baseSpeed / sizeScale,
		SpaceVect(baseRadius * sizeScale, 0.0),
		0.0,
		bullet_damage(to_float(baseDamage*sizeScale)),
		"starbowBreak"+boost::lexical_cast<string>(angle+1),
	};
}

bool StarbowBreak::spawnBullet(int angle, bool left)
{
	double _angle = agent->getAngle() + angleStep * angle * (left ? 1.0 : -1.0);
	SpaceVect pos = agent->getPos() + SpaceVect::ray(launchDist, _angle);
	float cost = baseCost * radiusScales[angle];
	bool fired = false;

	if (agent->getAttribute(Attribute::stamina) >= cost) {
		fired = agent->bulletImplCheckSpawn<StarbowBreakBullet>(
			Bullet::makeParams(pos,_angle), 
			makeSharedCopy(generateProps(angle))
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
	double attackSpeed = agent->getAttribute(Attribute::attackSpeed);
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

Catadioptric::Catadioptric(Agent *const agent, int level) :
	FirePattern(agent)
{}

bool Catadioptric::spawnTail(SpaceFloat angleOffset)
{
	bool fired = false;
	double _angle = agent->getAngle() + angleOffset;
	SpaceVect pos = agent->getPos() + SpaceVect::ray(1.0, _angle);

	fired |= agent->bulletImplCheckSpawn<CatadioptricBullet>(
		Bullet::makeParams(pos, _angle),
		app::getBullet("catadioptricBullet1")
	).isFuture();

	for_irange(i, 0, secondaryBulletCount)
	{
		auto baseProps = app::getBullet("catadioptricBullet2");
		auto props = make_shared<bullet_properties>();
		*props = *baseProps;

		SpaceFloat variation = secondarySpeedVariation * 2.0;
		SpaceFloat step = variation / (secondaryBulletCount - 1);
		SpaceFloat actualAngle = _angle + getSpace()->getRandomFloat(-1.0f, 1.0f)*angleSpread / 4.0;
		props->speed += -secondarySpeedVariation + step*i;

		fired |= agent->bulletImplCheckSpawn<CatadioptricBullet>(
			Bullet::makeParams(pos, actualAngle),
			props
		).isFuture();
	}

	for_irange(i, 0, tertiaryBulletCount)
	{
		auto baseProps = app::getBullet("catadioptricBullet3");
		auto props = make_shared<bullet_properties>();
		*props = *baseProps;

		SpaceFloat variation = tertiarySpeedVariation * 2.0;
		SpaceFloat step = variation / (tertiaryBulletCount - 1);
		SpaceFloat actualAngle = _angle + getSpace()->getRandomFloat(-1.0f, 1.0f)*angleSpread / 2.0;

		props->speed += -tertiarySpeedVariation + step * i;

		fired |= agent->bulletImplCheckSpawn<CatadioptricBullet>(
			Bullet::makeParams(pos,actualAngle),
			props
		).isFuture();
	}

	return fired;
}

bool Catadioptric::fire()
{
	bool fired = false;
	SpaceFloat spread = angleSpread * 2.0;
	SpaceFloat angleStep = spread / (tailCount - 1);

	for_irange(i, 0, tailCount)
	{
		fired |= spawnTail( -angleSpread + angleStep*i);
	}

	return fired;
}

const array<ScarletDaggerPattern::properties, ScarletDaggerPattern::levelsCount> ScarletDaggerPattern::props = {
//	ScarletDaggerPattern::properties{0.0, 2.0f / 3.0f, 1},
	ScarletDaggerPattern::properties{float_pi / 12.0, 0.5f, 3},
	ScarletDaggerPattern::properties{float_pi / 8.0, 0.5f, 5},
};

ScarletDaggerPattern::ScarletDaggerPattern(Agent *const agent, int level) :
	FirePattern(agent),
	MultiBulletSpreadPattern(
		agent,
		props[level].fireInterval,
		props[level].spreadAngle,
		props[level].bulletCount
	),
	BulletImplPattern<BulletImpl>(app::getBullet("scarletDagger")),
	level(level)
{}

FlandreBigOrbPattern::FlandreBigOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("flandreBigOrb1"))
{
}

FlandreFastOrbPattern::FlandreFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("flandreFastOrb1"))
{
}

const float FlandreWideAnglePattern1::cooldown = 0.25f;

FlandreWideAnglePattern1::FlandreWideAnglePattern1(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 8.0, 3),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("flandreFastOrb1"))
{}

const float FlandreWideAnglePattern2::cooldown = 0.25f;

FlandreWideAnglePattern2::FlandreWideAnglePattern2(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 4.0, 5),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("flandreFastOrb1"))
{}

RumiaFastOrbPattern::RumiaFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("rumiaFastOrb1"))
{}

const float RumiaParallelPattern::cooldown = 1.0f / 6.0f;

RumiaParallelPattern::RumiaParallelPattern(Agent *const agent) :
	MultiBulletParallelPattern(agent, cooldown, 1.0, 3),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("rumiaFastOrb1"))
{}

CirnoLargeIceBulletPattern::CirnoLargeIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("cirnoLargeIceBullet"))
{}

CirnoSmallIceBulletPattern::CirnoSmallIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(app::getBullet("cirnoSmallIceBullet"))
{}
