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
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"

FreezeStatusEffect::FreezeStatusEffect(GObject* agent) :
	MagicEffect(agent, 0.0f)
{}

void FreezeStatusEffect::init()
{
	Player* _player = dynamic_cast<Player*>(agent);

	agent->addGraphicsAction(freezeEffectAction());

	agent->setFrozen(true);

	if (_player) {
		_player->setFiringSuppressed(true);
		_player->setMovementSuppressed(true);
	}
}

void FreezeStatusEffect::update()
{
	timerIncrement(timer);

	if (timer >= 5.0f) {
		crntState = state::ending;
	}

	else {
		ai::applyDesiredVelocity(agent, SpaceVect::zero, agent->getMaxAcceleration());
	}
}

void FreezeStatusEffect::end()
{
	Player* _player = dynamic_cast<Player*>(agent);

	//Stop sprite effect, assuming the effect ended early.
	agent->stopGraphicsAction(cocos_action_tag::freeze_status);
	agent->addGraphicsAction(freezeEffectEndAction());

	agent->setFrozen(false);

	if (_player) {
		_player->setFiringSuppressed(false);
		_player->setMovementSuppressed(false);
	}
}

DarknessCurseEffect::DarknessCurseEffect(Agent* agent) :
	MagicEffect(agent, 0.0f),
	agent(agent)
{
}

void DarknessCurseEffect::init()
{
	agent->setInhibitSpellcasting(true);
	agent->stopSpell();
	agent->addGraphicsAction(darknessCurseFlickerTintAction());
}

void DarknessCurseEffect::update()
{
	auto& as = *agent->getAttributeSystem();
	if (as[Attribute::darknessDamage] <= 0.0f) {
		crntState = state::ending;
	}
	as.timerDecrement(Attribute::darknessDamage, 9.0f);
}

void DarknessCurseEffect::end()
{
	agent->setInhibitSpellcasting(false);
	agent->stopGraphicsAction(cocos_action_tag::darkness_curse);
}

RedFairyStress::RedFairyStress(Agent* agent) :
	MagicEffect(agent, 1.0f),
	agent(agent)
{
	baseAttackSpeed = agent->getAttribute(Attribute::attackSpeed);
}

void RedFairyStress::update()
{
	auto& as = *agent->getAttributeSystem();

	as.set(
		Attribute::attackSpeed,
		baseAttackSpeed + max(25.0f, as[Attribute::stress]) / 25.0f
	);
}

void RedFairyStress::end()
{
	agent->getAttributeSystem()->set(Attribute::attackSpeed, baseAttackSpeed);
}

GhostProtection::GhostProtection(Agent* agent) :
	MagicEffect(agent, 1.0f),
	agent(agent)
{
}

void GhostProtection::init()
{
	crntHP = agent->getAttribute(Attribute::hp);
}

void GhostProtection::update()
{
	float _hp = agent->getAttribute(Attribute::hp);
	float delta = crntHP - _hp;

	accumulator += delta / agent->getAttribute(Attribute::maxHP) * 12.5f;

	if (accumulator >= 1.0f && agent->getAttribute(Attribute::hitProtection) == 0.0f)
	{
		agent->addGraphicsAction(flickerAction(0.25f, 5.0f, 128));
		agent->getAttributeSystem()->setTimedProtection(5.0f);

		accumulator = 0.0f;
	}
	if(crntHP != _hp)
		crntHP = _hp;
	timerDecrement(accumulator, 0.5f);
}

BulletSpeedFromHP::BulletSpeedFromHP(
	Agent* agent,
	float_pair debuffRange,
	float_pair buffRange,
	float maxDebuff,
	float maxBuff
) :
	MagicEffect(agent, 1.0f),
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
