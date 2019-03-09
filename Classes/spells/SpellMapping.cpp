//
//  SpellMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "PatchouliSpell.hpp"
#include "PlayerSpell.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"

#define entry(name,cls) {name, createDesc<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

template<typename T>
constexpr shared_ptr<SpellDesc> createDesc()
{
	return make_shared<SpellDescImpl<T>>();
}

const unordered_map<string, shared_ptr<SpellDesc>> Spell::spellDescriptors = {
	entry_same(DarknessSignDemarcation),
	entry_same(DarknessSignDemarcation2),
	entry_same(FireStarburst),
	entry_same(FlameFence),
	entry_same(IllusionDial),
	entry_same(NightSignPinwheel),
	entry_same(PlayerBatMode),
	entry_same(PlayerCounterClock),
	entry_same(PlayerDarkMist),
	entry_same(PlayerIceShield),
	entry_same(StarlightTyphoon),
	entry_same(Whirlpool1),
	entry_same(Whirlpool2),
};
