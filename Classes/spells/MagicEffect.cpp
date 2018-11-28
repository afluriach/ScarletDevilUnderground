//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "AIMixins.hpp"
#include "App.h"
#include "GObject.hpp"
#include "MagicEffect.hpp"
#include "object_ref.hpp"

MagicEffect::MagicEffect(gobject_ref target, float magnitude) :
target(target),
magnitude(magnitude),
crntState(state::created)
{}

bool MagicEffect::combine(shared_ptr<MagicEffect> rhs) {
	MagicEffect* _rhs = rhs.get();

	if(_rhs && typeid(this) == typeid(_rhs)){
		magnitude += _rhs->magnitude;
		return true;
	}
	return false;
}

FrostStatusEffect::FrostStatusEffect(gobject_ref target, float magnitude) :
	MagicEffectImpl<FrostStatusEffect>(target, magnitude)
{}

void FrostStatusEffect::init()
{
	//apply attribute debuff, if applicable
}

void FrostStatusEffect::update()
{
	//if magnitude of effect is high effect, convert to Freeze effect

	if (magnitude >= 1.0f)
	{
		crntState = state::ending;
		target.get()->addMagicEffect(make_shared<FreezeStatusEffect>(target, 3.0f));
	}
}

void FrostStatusEffect::end()
{
	//remove attribute debuff, if applicable
}

FreezeStatusEffect::FreezeStatusEffect(gobject_ref target, float magnitude) :
	MagicEffectImpl<FreezeStatusEffect>(target, magnitude)
{}

void FreezeStatusEffect::init()
{
	GObject* _target = target.get();
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	if (_target->sprite){
		spriteEffectAction = TintTo::createRecursive(0.5f, Color3B(64, 64, 255));
		_target->sprite->runAction(spriteEffectAction);
	}

	if (smo) {
		smo->setFrozen(true);
	}
}

void FreezeStatusEffect::update()
{
	magnitude -= App::secondsPerFrame;

	if (magnitude <= 0.0f)
		crntState = state::ending;
}

void FreezeStatusEffect::end()
{
	GObject* _target = target.get();
	StateMachineObject* smo = dynamic_cast<StateMachineObject*>(_target);

	//Stop sprite effect, assuming the effect ended early.
	if (_target->sprite) {
		_target->sprite->stopAction(spriteEffectAction);
		_target->sprite->runAction(TintTo::createRecursive(0.5f, Color3B(255, 255, 255)));
	}

	if (smo) {
		smo->setFrozen(false);
	}
}
