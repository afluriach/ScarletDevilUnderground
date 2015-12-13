//
//  ObjectMapping.cpp
//  FlansBasement
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Block.hpp"
#include "Bullet.hpp"
#include "Flower.h"
#include "Glyph.hpp"
#include "CollectGlyph.hpp"

#include "Marisa.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
static GObject::AdapterType consAdapter()
{
    return [](const ValueMap& args) -> GObject* { return new T(args); };
}

#define entry(name,cls) (name, consAdapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

const unordered_map<string,GObject::AdapterType> GObject::adapters =
    boost::assign::map_list_of
    entry_same(Block)
    entry_same(FireBullet)
    entry_same(WaterBullet)
    entry_same(Flower)
    entry_same(Glyph)
    entry_same(CollectGlyph)

    entry_same(Marisa)
    entry_same(Patchouli)
    entry_same(Player);
