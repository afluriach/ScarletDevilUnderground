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

const float StarbowBreak::baseDamage = 2.0f;
const float StarbowBreak::baseFireInterval = 1.0f / 5.0f;
const array<float, StarbowBreak::anglesCount> StarbowBreak::angleIntervalScales = {
	5.0f / 6.0f,
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
	1.0,
	0.5,
	2.0 / 3.0,
	0.25,
	1.0 / 3.0
};

const array<Color3B, StarbowBreak::anglesCount> StarbowBreak::colors = {
	hsv3B(0.0f,0.5f,1.0f),
	hsv3B(48.0f,0.5f,1.0f),
	hsv3B(120.0f,0.5f,1.0f),
	hsv3B(180.0f,0.5f,1.0f),
	hsv3B(315.0f,0.5f,1.0f),
};

StarbowBreak::StarbowBreak(Agent *const agent) :
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
		baseRadius * sizeScale,
		hp_damage_map(baseDamage*sizeScale),
		0.83,
		"sprites/starbow_break_bullet.png",
		colors[angle]
	};
}

bool StarbowBreak::spawnBullet(int angle, bool left)
{
	double _angle = agent->getAngle() + angleStep * angle * (left ? 1.0 : -1.0);
	SpaceVect pos = agent->getPos() + SpaceVect::ray(launchDist, _angle);

	return agent->bulletValueImplCheckSpawn<StarbowBreakBullet>(pos, _angle, generateProps(angle)).isFuture();
}

bool StarbowBreak::fire()
{
	bool canFire = false;
	for (float t : timers) {
		if (t <= 0.0f) {
			canFire = true;
			break;
		}
	}

	if (!canFire)
		return false;

	if (timers[0] <= 0.0f && spawnBullet(0,false))
	{
		timers[0] = baseFireInterval;
	}

	//angle zero represents forward direction (single bullet)
	//others represent pair of side-angle bullets
	for_irange(i, 1, anglesCount)
	{
		if (timers[i] <= 0.0f) {
			bool fired = false;
			fired |= spawnBullet(i, false);
			fired |= spawnBullet(i, true);

			if (fired)
				timers[i] = baseFireInterval;
		}
	}

	return true;
}

void StarbowBreak::update()
{
	for_irange(i, 0, anglesCount) {
		timerDecrement(timers[i], angleIntervalScales[i]);
	}
}

const array<ScarletDaggerPattern::properties, ScarletDaggerPattern::levelsCount> ScarletDaggerPattern::props = {
	ScarletDaggerPattern::properties{0.0, 2.0f / 3.0f, 1},
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
	level(level)
{}

FlandreBigOrbPattern::FlandreBigOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::flandreBigOrb1)
{
}

FlandreFastOrbPattern::FlandreFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent)
{
}

FlandreWhirlShotPattern::FlandreWhirlShotPattern(Agent *const agent) :
	RadiusPattern(agent, 1.0f, 8),
	FirePattern(agent)
{
}

const float FlandreWideAnglePattern1::cooldown = 0.25f;

FlandreWideAnglePattern1::FlandreWideAnglePattern1(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 8.0, 3),
	FirePattern(agent)
{}

const float FlandreWideAnglePattern2::cooldown = 0.25f;

FlandreWideAnglePattern2::FlandreWideAnglePattern2(Agent *const agent) :
	MultiBulletSpreadPattern(agent, cooldown, float_pi / 4.0, 5),
	FirePattern(agent)
{}

RumiaFastOrbPattern::RumiaFastOrbPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::rumiaFastOrb1)
{}

const float RumiaParallelPattern::cooldown = 1.0f / 6.0f;

RumiaParallelPattern::RumiaParallelPattern(Agent *const agent) :
	MultiBulletParallelPattern(agent, cooldown, 1.0, 3),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::rumiaFastOrb1)
{}

CirnoLargeIceBulletPattern::CirnoLargeIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::cirnoLargeIceBullet)
{}

CirnoSmallIceBulletPattern::CirnoSmallIceBulletPattern(Agent *const agent) :
	SingleBulletFixedIntervalPattern(agent),
	FirePattern(agent),
	PlayerBulletImplPattern(&PlayerBulletImpl::cirnoSmallIceBullet)
{}
