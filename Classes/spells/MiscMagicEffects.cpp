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
#include "App.h"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"

FreezeStatusEffect::FreezeStatusEffect(gobject_ref target) :
	MagicEffect(target, 0.0f)
{}

void FreezeStatusEffect::init()
{
	GObject* _target = target.get();
	Player* _player = dynamic_cast<Player*>(_target);
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	if (_target->spriteID != 0){
		_target->space->runSpriteAction(_target->spriteID, freezeEffectAction());
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
		_target->space->stopSpriteAction(_target->spriteID, cocos_action_tag::freeze_status);
		_target->space->runSpriteAction(_target->spriteID, freezeEffectEndAction());
	}

	if (smo) {
		smo->setFrozen(false);
	}

	if (_player) {
		_player->setFiringSuppressed(false);
		_player->setMovementSuppressed(false);
	}
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