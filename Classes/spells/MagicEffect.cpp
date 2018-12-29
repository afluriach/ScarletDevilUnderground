//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIMixins.hpp"
#include "App.h"
#include "GObject.hpp"
#include "MagicEffect.hpp"
#include "object_ref.hpp"
#include "Player.hpp"

MagicEffect::MagicEffect(gobject_ref target, float magnitude) :
target(target),
magnitude(magnitude),
crntState(state::created)
{}

FreezeStatusEffect::FreezeStatusEffect(gobject_ref target) :
	MagicEffectImpl<FreezeStatusEffect>(target, 0.0f)
{}

void FreezeStatusEffect::init()
{
	GObject* _target = target.get();
	Player* _player = dynamic_cast<Player*>(_target);
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	if (_target->sprite){
		spriteEffectAction = TintTo::createRecursive(0.5f, Color3B(64, 64, 255));
		_target->sprite->runAction(spriteEffectAction);
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
	Agent* _target = dynamic_cast<Agent*>(target.get());

	if (!_target) {
		crntState = state::ending;
		return;
	}

	if (_target->getAttribute(Attribute::iceDamage) < 75.0f) {
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
	if (_target->sprite) {
		_target->sprite->stopAction(spriteEffectAction);
		_target->sprite->runAction(TintTo::createRecursive(0.5f, Color3B(255, 255, 255)));
	}

	if (smo) {
		smo->setFrozen(false);
	}

	if (_player) {
		_player->setFiringSuppressed(false);
		_player->setMovementSuppressed(false);
	}
}
