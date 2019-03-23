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

const float StarbowBreak::baseFireInterval = 1.0f / 4.0f;
const float StarbowBreak::fireIntervalVariation = 1.0f / 6.0f;
const double StarbowBreak::fireIntervalZPos = -2.0;

const double StarbowBreak::baseSpeed = 6.0;
const double StarbowBreak::speedVariation = 2.0;
const double StarbowBreak::speedZPos = 0.0;

const double StarbowBreak::angleVariation = float_pi * 0.25;
const double StarbowBreak::angleZPos = 2.0;

const double StarbowBreak::baseRadius = 0.15;
const double StarbowBreak::radiusVariation = 0.1;
const double StarbowBreak::radiusZPos = 1.0;

StarbowBreak::StarbowBreak(Agent *const agent) :
	FirePattern(agent)
{}

bool StarbowBreak::fire()
{
	SpaceFloat angleOffsetNoiseValue = noiseModel.GetValue(cos(noisePos), sin(noisePos), angleZPos);
	SpaceFloat angleOffset = angleVariation * angleOffsetNoiseValue*angleOffsetNoiseValue * (angleOffsetNoiseValue < 0.0 ? -1.0 : 1.0);
	SpaceFloat sizeScale = noiseModel.GetValue(cos(noisePos), sin(noisePos), radiusZPos);

	bullet_properties props = {
		0.1,
		baseSpeed + speedVariation*noiseModel.GetValue(cos(noisePos), sin(noisePos), speedZPos),
		baseRadius + radiusVariation*sizeScale,
		0.83,
		"sprites/flandre_bullet.png",
		hp_damage_map(2.0f + 1.5f*sizeScale)
	};

	SpaceFloat angle = agent->getAngle() + angleOffset;
	SpaceVect pos = agent->getPos() + SpaceVect::ray(1.0, angle);

	return agent->bulletValueImplCheckSpawn<PlayerBulletValueImpl>(pos,angle,props).isFuture();
}

void StarbowBreak::update()
{
	FirePattern::update();
	noisePos = canonicalAngle(noisePos + float_pi * App::secondsPerFrame);
}

float StarbowBreak::getCooldownTime()
{
	return baseFireInterval + fireIntervalVariation * noiseModel.GetValue(cos(noisePos), sin(noisePos), fireIntervalZPos);
}

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
