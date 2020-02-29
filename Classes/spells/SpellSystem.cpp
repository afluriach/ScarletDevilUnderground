//
//  SpellSystem.cpp
//  Koumachika
//
//  Created by Toni on 12/30/19.
//
//

#include "Prefix.h"

#include "Spell.hpp"
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

unsigned int SpellSystem::cast(local_shared_ptr<SpellDesc> desc, GObject* caster)
{
	if (!desc) {
		log("cast(): null SpellDescriptor!");
		return 0;
	}
	if (!caster) {
		log("cast(): null caster!");
		return 0;
	}

	if (!caster->applyInitialSpellCost(desc->getCost())) {
		return 0;
	}

	unsigned int id = nextID++;
	Spell* spell = desc->generate(caster, desc, id);

	spell->init();

	if (logSpells) {
		log("Spell %s (%u) created and initialized.", spell->getName(), spell->id);
	}

	if (spell->length != 0.0){
		spells.insert_or_assign(id, spell);
		objectSpells.insert(make_pair(caster, id));
		additions.push_back(spell);
	}
	else {
		if (logSpells) {
			log("Immediate spell %s (%u) deleted.", spell->getName(), spell->id);
		}
		allocator_delete(spell);
	}

	return id;
}

void SpellSystem::stopSpell(unsigned int id)
{
	if (!id) return;

	auto it = spells.find(id);
	if (it != spells.end()) {
		if (logSpells) {
			log("Spell %s (%u) stopped.", it->second->getName(), it->second->id);
		}

		it->second->end();
		removals.insert(it->second);

		eraseEntry(objectSpells, make_pair(it->second->caster, it->first));
		spells.erase(it);
	}
	else {
		log("stopSpell(): spell ID %u does not exist!", id);
	}
}

bool SpellSystem::isSpellActive(unsigned int id)
{
	if (!id) return false;

	auto it = spells.find(id);
	return it != spells.end();
}

void SpellSystem::onRemove(unsigned int id, Bullet* b)
{
	auto it = spells.find(id);
	if (it != spells.end()) {
		it->second->onBulletRemove(b);
	}
}

void SpellSystem::applyRemove(Spell* spell)
{
	updateSpells.erase(spell);
	allocator_delete(spell);
}

void SpellSystem::stopObjectSpells(GObject* obj)
{
	auto keysRange = objectSpells.equal_range(obj);
	vector<unsigned int> ids; 

	for (auto it = keysRange.first; it != keysRange.second; ++it)
		ids.push_back(it->second);

	for (auto id : ids) {
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
