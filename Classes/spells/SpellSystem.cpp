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

SpellSystem::SpellSystem(GSpace* gspace) :
	gspace(gspace)
{
}

unsigned int SpellSystem::cast(shared_ptr<SpellDesc> desc, GObject* caster)
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

	if (spell->length != 0.0){
		spells.insert_or_assign(id, spell);
		objectSpells.insert(make_pair(caster, id));
	}
	else {
		delete spell;
	}

	return id;
}

void SpellSystem::stopSpell(unsigned int id)
{
	if (!id) return;

	auto it = spells.find(id);
	if (it != spells.end()) {
		it->second->end();
		toRemove.push_back(id);
	}
	else {
		log("stopSpell(): spell ID % does not exist!", id);
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

void SpellSystem::applyRemove(unsigned int id)
{
	auto it = spells.find(id);
	if (it != spells.end()) {
		//erase object-spell entry
		eraseEntry(objectSpells, make_pair(it->second->caster, it->first));
		delete it->second;
		spells.erase(it);
	}
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

void SpellSystem::update()
{
	for (auto id : toRemove) {
		applyRemove(id);
	}
	toRemove.clear();

	for (auto it = spells.begin(); it != spells.end(); ++it) {
		if (!it->second->caster->applyOngoingSpellCost(it->second->getDescriptor()->getCost())) {
			stopSpell(it->second->id);
		}
		else {
			it->second->runUpdate();
		}
	}
}
