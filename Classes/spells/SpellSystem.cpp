//
//  SpellSystem.cpp
//  Koumachika
//
//  Created by Toni on 12/30/19.
//
//

#include "Prefix.h"

#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"

bool spellCompareID::operator()(const Spell* left, const Spell* right) const {
	return left->getID() < right->getID();
}

SpellSystem::SpellSystem(GSpace* gspace) :
	gspace(gspace)
{
}

SpellSystem::~SpellSystem()
{
	applyRemovals();
}

local_shared_ptr<Spell> SpellSystem::cast(const SpellDesc* desc, GObject* caster)
{
	if (!desc) {
		log0("null SpellDescriptor!");
		return nullptr;
	}
	if (!caster) {
		log0("null caster!");
		return nullptr;
	}

	if (!caster->applyInitialSpellCost(desc->getCost())) {
		return nullptr;
	}

	unsigned int id = nextID++;
	local_shared_ptr<Spell> spell = desc->generate(caster, id);

	spell->init();

	if (logSpells) {
		log2("Spell %s (%u) created and initialized.", spell->getName(), spell->id);
	}

	if (spell->descriptor->params.length != 0.0){
		spells.insert_or_assign(id, spell);
		objectSpells.insert(make_pair(caster, spell.get()));
		additions.push_back(spell.get());
	}
	else {
		if (logSpells) {
			log2("Immediate spell %s (%u) deleted.", spell->getName(), spell->id);
		}
	}

	return spell;
}

void SpellSystem::stopSpell(unsigned int id)
{
	if (!id) return;

	auto it = spells.find(id);
	if (it != spells.end()) {
		if (logSpells) {
			log2("Spell %s (%u) stopped.", it->second->getName(), it->second->id);
		}

		it->second->end();
		it->second->active = false;
		removals.insert(it->second.get());

		eraseEntry(objectSpells, make_pair(it->second->caster, it->second.get()));
		spells.erase(it);
	}
	else {
		log1("spell ID %u does not exist!", id);
	}
}

void SpellSystem::applyRemove(Spell* spell)
{
	updateSpells.erase(spell);
}

void SpellSystem::stopObjectSpells(GObject* obj)
{
	auto keysRange = objectSpells.equal_range(obj);

	//This is required, as the keysRange iterator is invalidated once stopSpell is called.
	list<unsigned int, local_allocator<unsigned int>> toRemove;

	for (auto it = keysRange.first; it != keysRange.second; ++it) {
		toRemove.push_back(it->second->id);
	}

	for (auto id : toRemove) {
		stopSpell(id);
	}
}

void SpellSystem::applyRemovals()
{
	for (auto spell : removals) {
		applyRemove(spell);
	}
	removals.clear();
}

void SpellSystem::update()
{
	for (auto spell : additions) {
		updateSpells.insert(spell);
	}
	additions.clear();

	applyRemovals();

	for (auto it = updateSpells.begin(); it != updateSpells.end(); ++it) {
		Spell* crnt = *it;
		if (!crnt->caster->applyOngoingSpellCost(crnt->getCost())) {
			stopSpell(crnt->id);
		}
		else {
			crnt->runUpdate();
		}
	}
}
