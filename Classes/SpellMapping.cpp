//
//  SpellMapping.cpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "Spell.hpp"

#define entry(name,cls) (name, adapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

template<typename T>
Spell::AdapterType adapter()
{
    return [](GObject* caster, const ValueMap& args) -> shared_ptr<Spell> {return make_shared<T>(caster, args);};
}

const unordered_map<string,Spell::AdapterType> Spell::adapters = boost::assign::map_list_of
    entry_same(FireStarburst)
    entry_same(FlameFence)
;

const set<string> Spell::scripts = boost::assign::list_of
    ("Splash")
    ("Test")
;