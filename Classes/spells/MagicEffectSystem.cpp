//
//  MagicEffectSystem.cpp
//  Koumachika
//
//  Created by Toni on 6/24/19.
//
//

#include "Prefix.h"

#include "MagicEffectSystem.hpp"

bool timedEntry::operator>(const timedEntry& rhs) const
{
	if (endTime > rhs.endTime)
		return true;
	else if (endTime < rhs.endTime)
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

MagicEffectSystem::~MagicEffectSystem()
{
}

unsigned int MagicEffectSystem::applyEffect(
	GObject* target,
	const MagicEffectDescriptor* effect,
	effect_attributes attr
){
	unsigned int id = 0;

	if (effect->canApply(target, attr)) {
		id = nextID++;
		effect_params params = { target, id, effect->getFlags(), effect, attr };
		addEffect(effect->generate(params));
	}

	return id;
}

void MagicEffectSystem::addEffect(local_shared_ptr<MagicEffect> effect)
{
	if(isValidConfig(effect.get()))
		magicEffectsToAdd.push_back(effect);
}

void MagicEffectSystem::removeEffect(local_shared_ptr<MagicEffect> effect)
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

local_shared_ptr<MagicEffect> MagicEffectSystem::getByID(unsigned int id)
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

			if (newEffect->length > 0.0f) {
				addTimedEntry(newEffect);
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
	SpaceFloat crntTime = gspace->getTime();

	while (!timedRemovals.empty() && timedRemovals.top().endTime <= crntTime)
	{
		removeEffect(timedRemovals.top().effect->id);
		timedRemovals.pop();
	}
}

void MagicEffectSystem::addTimedEntry(MagicEffect* newEffect)
{
	SpaceFloat endTime = gspace->getTime() + newEffect->length;
	timedRemovals.push(timedEntry{ endTime, newEffect });
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
