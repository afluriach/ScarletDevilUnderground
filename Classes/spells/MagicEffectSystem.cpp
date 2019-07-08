//
//  MagicEffectSystem.cpp
//  Koumachika
//
//  Created by Toni on 6/24/19.
//
//

#include "Prefix.h"

#include "MagicEffectSystem.hpp"

MagicEffectSystem::MagicEffectSystem()
{
}

void MagicEffectSystem::addEffect(shared_ptr<MagicEffect> effect)
{
	magicEffectsToAdd.push_back(effect);
}

void MagicEffectSystem::removeObjectEffects(GObject* obj)
{
	auto it = effectObjects.find(obj);
	if (it != effectObjects.end()) {
		for (auto entry : it->second) {
			magicEffectsToRemove.push_back(entry);
		}
	}

	applyRemove();
}

void MagicEffectSystem::applyAdd()
{
	for (auto it = magicEffectsToAdd.begin(); it != magicEffectsToAdd.end(); ++it)
	{
		shared_ptr<MagicEffect> newEffect = *it;
		GObject* obj = newEffect->agent;

		emplaceIfEmpty(effectObjects, obj);
		effectObjects.at(obj).push_back(newEffect);

		if (!newEffect || newEffect.get()->crntState != MagicEffect::state::created) {
			log("GObject::addMagicEffect: invalid magic effect");
			continue;
		}

		magicEffects.push_back(newEffect);
	}

	magicEffectsToAdd.clear();
}

void MagicEffectSystem::update()
{
	applyAdd();

	for (auto it = magicEffects.begin(); it != magicEffects.end(); ++it)
	{
		MagicEffect* _crntEffect = (*it).get();

		switch (_crntEffect->crntState)
		{
		case MagicEffect::state::created:
			_crntEffect->init();
			_crntEffect->crntState = MagicEffect::state::active;
			//intentional fall through - update will be first called on the same frame as init
		case MagicEffect::state::active:
			_crntEffect->update();
			break;
		case MagicEffect::state::ending:
			_crntEffect->end();
			_crntEffect->crntState = MagicEffect::state::expired;
			magicEffectsToRemove.push_back(*it);
			break;
		default:
			log("Invalid MagicEffect state %d", to_int(_crntEffect->crntState));
			break;
		}
	}
}

void MagicEffectSystem::applyRemove()
{
	for (auto it = magicEffectsToRemove.begin(); it != magicEffectsToRemove.end(); ++it)
	{
		magicEffects.remove(*it);
	}
	magicEffectsToRemove.clear();
}
