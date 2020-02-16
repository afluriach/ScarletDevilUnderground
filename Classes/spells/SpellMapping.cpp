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
#include "RumiaSpells.hpp"
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

const unordered_map<string, local_shared_ptr<SpellDesc>> Spell::spellDescriptors = {
	entry_same(BlueFairyBomb),
	entry_same(DarknessSignDemarcation),
	entry_same(DarknessSignDemarcation2),
	entry_same(LavaeteinnSpell),
	entry_same(NightSignPinwheel),
	entry_same(PlayerBatMode),
	entry_same(PlayerCounterClock),
	entry_same(PlayerIceShield),
	entry_same(PlayerScarletRose),
	entry_same(TorchDarkness),
};

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
