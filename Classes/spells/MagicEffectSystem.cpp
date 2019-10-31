//
//  MagicEffectSystem.cpp
//  Koumachika
//
//  Created by Toni on 6/24/19.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "MagicEffectSystem.hpp"

bool timedEntry::operator>(const timedEntry& rhs) const
{
	if (endFrame > rhs.endFrame)
		return true;
	else {
		return effect > rhs.effect;
	}
}

MagicEffectSystem::MagicEffectSystem(GSpace* gspace) :
	timedRemovals(),
	gspace(gspace)
{
}

void MagicEffectSystem::addEffect(MagicEffect* effect)
{
	if(isValidConfig(effect))
		magicEffectsToAdd.push_back(effect);
	else
		delete effect;
}

void MagicEffectSystem::removeEffect(MagicEffect* effect)
{
	magicEffectsToRemove.push_back(effect);
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

bool MagicEffectSystem::hasScriptedEffect(GObject* obj, string clsName)
{
	auto it = effectObjects.find(obj);
	if (it == effectObjects.end()) return false;

	for (auto entry : it->second) {
		if (auto script = dynamic_cast<ScriptedMagicEffect*>(entry)) {
			if(script->getClsName() == clsName) return true;
		}
	}
	return false;
}

void MagicEffectSystem::applyAdd()
{
	for (auto it = magicEffectsToAdd.begin(); it != magicEffectsToAdd.end(); ++it)
	{
		MagicEffect* newEffect = *it;
		GObject* obj = newEffect->agent;

		if (!newEffect || newEffect->crntState != MagicEffect::state::created) {
			log("invalid magic effect not in created state");
			continue;
		}

		newEffect->init();
		newEffect->crntState = MagicEffect::state::active;

		if (!newEffect->isImmediate()) {
			emplaceIfEmpty(effectObjects, obj);
			effectObjects.at(obj).push_back(newEffect);

			magicEffects.push_back(newEffect);

			if (newEffect->isActive()) {
				updateEffects.push_back(newEffect);
			}

			if (newEffect->isTimed()) {
				float length = newEffect->length;
				if (length > 0.0f) {
					unsigned int endFrame = gspace->getFrame() + app::params.framesPerSecond * length;
					
					timedRemovals.push(timedEntry{ endFrame, newEffect });
				}
			}
		}
	}

	magicEffectsToAdd.clear();
}

void MagicEffectSystem::update()
{
	applyAdd();

	for (auto it = updateEffects.begin(); it != updateEffects.end(); ++it)
	{
		MagicEffect* _crntEffect = (*it);
		_crntEffect->update();
	}

	processTimedRemovals();
	applyRemove();
}

void MagicEffectSystem::processTimedRemovals()
{
	unsigned int crntFrame = gspace->getFrame();

	while (!timedRemovals.empty() && timedRemovals.top().endFrame <= crntFrame)
	{
		removeEffect(timedRemovals.top().effect);
		timedRemovals.pop();
	}
}

void MagicEffectSystem::applyRemove()
{
	for (auto it = magicEffectsToRemove.begin(); it != magicEffectsToRemove.end(); ++it)
	{
		(*it)->crntState = MagicEffect::state::ending;
		(*it)->end();

		magicEffects.remove(*it);

		if ( (*it)->isActive() ) {
			updateEffects.remove(*it);
		}

		auto effectIt = effectObjects.find((*it)->agent);
		if (effectIt != effectObjects.end()) {
			effectIt->second.remove(*it);
		}

		delete (*it);
	}
	magicEffectsToRemove.clear();
}

bool MagicEffectSystem::isValidConfig(MagicEffect* effect)
{
	if (bool_int(effect->_flags[to_size_t(MagicEffect::flags::immediate)]) +
		bool_int(effect->_flags[to_size_t(MagicEffect::flags::indefinite)]) +
		bool_int(effect->_flags[to_size_t(MagicEffect::flags::timed)]) != 1
	){
		log("Invalid time properties; exactly one time flag should be set.");
		return false;
	}

	return true;
}
