//
//  SpellMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "EnemySpell.hpp"
#include "PlayerSpell.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

#define entry(name,cls) {name, createDesc<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)
#define script_entry(cls) { #cls, make_local_shared<ScriptedSpellDescriptor>(#cls)}

template<typename T>
constexpr local_shared_ptr<SpellDesc> createDesc()
{
	return make_local_shared<SpellDescImpl<T>>();
}

void Spell::initDescriptors() {
	spellDescriptors.insert(entry_same(BlueFairyBomb));
	spellDescriptors.insert(entry_same(LavaeteinnSpell));
	spellDescriptors.insert(entry_same(PlayerBatMode));
	spellDescriptors.insert(entry_same(PlayerCounterClock));
	spellDescriptors.insert(entry_same(PlayerIceShield));
	spellDescriptors.insert(entry_same(PlayerScarletRose));
	spellDescriptors.insert(entry_same(TorchDarkness));
}

unordered_map<string, local_shared_ptr<SpellDesc>> Spell::spellDescriptors;

const vector<string> Spell::playerSpells = {
	"DarkMist",
	"PlayerBatMode",
	"PlayerCounterClock",
	"PlayerIceShield",
	"PlayerScarletRose",
};

const vector<string> Spell::playerPowerAttacks = {
	"LavaeteinnSpell",
};
