//
//  SpellMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

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
	entry_same(FireStarburst),
	entry_same(FlameFence),
	entry_same(IllusionDial),
	entry_same(PlayerBatMode),
	entry_same(PlayerDarkMist),
	entry_same(StarlightTyphoon)
};
