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
	else if (endFrame < rhs.endFrame)
		return false;
	else
		return effect->id > rhs.effect->id;
}

MagicEffectSystem::MagicEffectSystem(GSpace* gspace) :
	timedRemovals(),
	gspace(gspace)
{
	nextID = 1;
}

bool MagicEffectSystem::applyEffect(GObject* target, shared_ptr<MagicEffectDescriptor> effect, float magnitude, float length)
{
	bool success = false;

	if (effect->canApply(target, magnitude, length)) {
		effect_params params = { target, nextID++, effect };
		addEffect(effect->generate(params, magnitude, length));
		success = true;
	}

	return success;
}

void MagicEffectSystem::addEffect(shared_ptr<MagicEffect> effect)
{
	if(isValidConfig(effect.get()))
		magicEffectsToAdd.push_back(effect);
}

void MagicEffectSystem::removeEffect(shared_ptr<MagicEffect> effect)
{
	removeEffect(effect->id);
}

void MagicEffectSystem::removeEffect(unsigned int id)
{
	magicEffectsToRemove.push_back(id);
}

void MagicEffectSystem::removeObjectEffects(GObject* obj)
{
	auto it = effectObjects.find(obj);
	if (it != effectObjects.end()) {
		for (auto entry : it->second) {
			magicEffectsToRemove.push_back(entry->id);
		}
	}

	applyRemove();
}

shared_ptr<MagicEffect> MagicEffectSystem::getByID(unsigned int id)
{
	return getOrDefault(magicEffects, id);
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
		MagicEffect* newEffect = it->get();
		GObject* obj = newEffect->target;

		if (!newEffect || newEffect->crntState != MagicEffect::state::created) {
			log("invalid magic effect not in created state");
			continue;
		}

		newEffect->init();
		newEffect->crntState = MagicEffect::state::active;

		if (!newEffect->isImmediate()) {
			emplaceIfEmpty(effectObjects, obj);
			effectObjects.at(obj).insert(newEffect);

			magicEffects.insert_or_assign(newEffect->id, *it);

			if (newEffect->isActive()) {
				updateEffects.insert(newEffect);
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
		removeEffect(timedRemovals.top().effect->id);
		timedRemovals.pop();
	}
}

void MagicEffectSystem::applyRemove()
{
	while (!magicEffectsToRemove.empty())
	{
		unsigned int targetID = magicEffectsToRemove.front();
		MagicEffect* crnt = getByID(targetID).get();
		magicEffectsToRemove.pop_front();

		if (!crnt) {
			log("MagicEffectSystem: attempt to remove invalid effect ID %d!", targetID);
			continue;
		}

		crnt->crntState = MagicEffect::state::ending;
		crnt->end();

		if ( crnt->isActive() ) {
			updateEffects.erase(crnt);
		}

		auto effectIt = effectObjects.find(crnt->target);
		if (effectIt != effectObjects.end()) {
			effectIt->second.erase(crnt);
		}

		//Typically, this will cause the shared refcount to go to zero, and the crnt effect will be deallocated.
		magicEffects.erase(crnt->id);
	}
}

bool MagicEffectSystem::isValidConfig(MagicEffect* effect)
{
	return effect->_flags != effect_flags::none;
}
