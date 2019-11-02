//
//  MiscMagicEffects.cpp
//  Koumachika
//
//  Created by Toni on 3/12/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "GObject.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"

BulletSpeedFromHP::BulletSpeedFromHP(
	Agent* agent,
	float_pair debuffRange,
	float_pair buffRange,
	float maxDebuff,
	float maxBuff
) :
	MagicEffect(agent, 0.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
	agent(agent),
	debuffRange(debuffRange),
	buffRange(buffRange),
	maxDebuff(maxDebuff),
	maxBuff(maxBuff)
{
	if (agent) {
		baseBulletSpeed = agent->getAttribute(Attribute::bulletSpeed);
	}
	else {
		log("BulletSpeedFromHP should only be attached to Agents.");
		crntState = state::expired;
	}
}

void BulletSpeedFromHP::update()
{
	auto& as = *agent->getAttributeSystem();
	float hpRatio = agent->getHealthRatio();
	float modifier = 0.0f;

	if (hpRatio >= buffRange.first) {
		float buffRatio = hpRatio >= buffRange.second ? 1.0f : (hpRatio - buffRange.first) / (buffRange.second - buffRange.first);
		modifier = maxBuff * buffRatio;
	}
	else if (hpRatio < debuffRange.second) {
		float debuffRatio = hpRatio <= buffRange.first ? 1.0f : (debuffRange.second - hpRatio) / (debuffRange.second - debuffRange.first);
		modifier = -1.0f * maxDebuff * debuffRatio;
	}

	as.set(Attribute::bulletSpeed, baseBulletSpeed + modifier);
}

void BulletSpeedFromHP::end()
{
	agent->getAttributeSystem()->set(Attribute::bulletSpeed, baseBulletSpeed);
}

DrainStaminaFromMovement::DrainStaminaFromMovement(Agent* agent) :
	MagicEffect(agent, 0.0f, 0.0f, enum_bitfield2(flags, indefinite, active)),
	agent(agent)
{
}

void DrainStaminaFromMovement::update()
{
	SpaceFloat dp = agent->getVel().length() * app::params.secondsPerFrame;
	agent->modifyAttribute(Attribute::stamina, -dp);
}
