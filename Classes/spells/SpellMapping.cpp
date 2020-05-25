//
//  SpellMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "PlayerSpell.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

#define entry(name,cls) {name, createDesc<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)
#define script_entry(cls) { #cls, make_local_shared<ScriptedSpellDescriptor>(#cls)}

template<typename T>
const SpellDesc* createDesc()
{
	return new SpellDescImpl<T>();
}

void Spell::initDescriptors() {
	spellDescriptors.insert(entry_same(LavaeteinnSpell));
	spellDescriptors.insert(entry_same(PlayerBatMode));
	spellDescriptors.insert(entry_same(PlayerCounterClock));
	spellDescriptors.insert(entry_same(PlayerIceShield));
}

unordered_map<string, const SpellDesc*> Spell::spellDescriptors;

const vector<string> Spell::playerSpells = {
	"DarkMist",
	"PlayerBatMode",
	"PlayerCounterClock",
	"PlayerIceShield",
};

const vector<string> Spell::playerPowerAttacks = {
	"LavaeteinnSpell",
};
