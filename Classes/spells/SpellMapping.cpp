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

#define entry(name,cls) (name, createDesc<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

template<typename T>
SpellDesc* createDesc()
{
	return new SpellDescImpl<T>();
}

const unordered_map<string,SpellDesc*> Spell::spellDescriptors = boost::assign::map_list_of
    entry_same(FireStarburst)
    entry_same(FlameFence)
    entry_same(StarlightTyphoon)

	entry_same(PlayerBatMode)
	entry_same(PlayerDarkMist)

    entry_same(IllusionDial)
;
