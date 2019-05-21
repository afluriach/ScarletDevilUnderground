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

RadiusEffect::RadiusEffect(gobject_ref target, DamageInfo damage, SpaceFloat radius, GType type) :
	MagicEffect(target),
	damage(damage),
	radius(radius),
	type(type)
{}

void RadiusEffect::update()
{
	radialEffectArea(target.get(), radius, type, damage);
}

FreezeStatusEffect::FreezeStatusEffect(gobject_ref target) :
	MagicEffect(target, 0.0f)
{}

void FreezeStatusEffect::init()
{
	GObject* _target = target.get();
	Player* _player = dynamic_cast<Player*>(_target);
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	if (_target->spriteID != 0){
		_target->space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			_target->spriteID,
			freezeEffectAction()
		);
	}

	if (smo) {
		smo->setFrozen(true);
	}

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
		ai::applyDesiredVelocity(target.get(), SpaceVect::zero, target.get()->getMaxAcceleration());
	}
}

void FreezeStatusEffect::end()
{
	GObject* _target = target.get();
	Player* _player = dynamic_cast<Player*>(_target);
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	//Stop sprite effect, assuming the effect ended early.
	if (_target->spriteID != 0) {
		_target->space->addGraphicsAction(
			&graphics_context::stopSpriteAction,
			_target->spriteID,
			cocos_action_tag::freeze_status
		);
		_target->space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			_target->spriteID,
			freezeEffectEndAction()
		);
	}

	if (smo) {
		smo->setFrozen(false);
	}

	if (_player) {
		_player->setFiringSuppressed(false);
		_player->setMovementSuppressed(false);
	}
}

DarknessCurseEffect::DarknessCurseEffect(gobject_ref target) :
	MagicEffect(target, 0.0f)
{
	agent = dynamic_cast<Agent*>(target.get());
	if(!agent){
		log("DarknessCurseEffect should only be attached to Agents.");
	}
}

void DarknessCurseEffect::init()
{
	agent->inhibitSpellcasting = true;
	agent->stopSpell();
	agent->space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		agent->spriteID,
		darknessCurseFlickerTintAction()
	);
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
	agent->inhibitSpellcasting = false;
	agent->space->addGraphicsAction(
		&graphics_context::stopSpriteAction,
		agent->spriteID,
		cocos_action_tag::darkness_curse
	);
}

RedFairyStress::RedFairyStress(object_ref<Agent> _agent) :
	MagicEffect(_agent, 1.0f),
	agent(_agent.get())
{
	if (agent) {
		baseAttackSpeed = agent->getAttribute(Attribute::attackSpeed);
	}
	else {
		log("RedFairyStress should only be attached to Agents.");
		crntState = state::expired;
	}
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

GhostProtection::GhostProtection(object_ref<Agent> _agent) :
	MagicEffect(_agent, 1.0f),
	agent(_agent.get())
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
		agent->space->addGraphicsAction(
			&graphics_context::runSpriteAction,
			agent->spriteID,
			flickerAction(0.25f, 5.0f, 128)
		);
		agent->getAttributeSystem()->setTimedProtection(5.0f);

		accumulator = 0.0f;
	}
	if(crntHP != _hp)
		crntHP = _hp;
	timerDecrement(accumulator, 0.5f);
}

BulletSpeedFromHP::BulletSpeedFromHP(
	object_ref<Agent> _agent,
	float_pair debuffRange,
	float_pair buffRange,
	float maxDebuff,
	float maxBuff
) :
	MagicEffect(_agent, 1.0f),
	agent(_agent.get()),
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
