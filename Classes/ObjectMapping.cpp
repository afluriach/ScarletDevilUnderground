//
//  ObjectMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Block.hpp"
#include "Bullet.hpp"
#include "Flower.h"
#include "Glyph.hpp"
#include "Torch.hpp"
#include "CollectGlyph.hpp"

#include "Marisa.hpp"
#include "Patchouli.hpp"
#include "Reimu.hpp"

#include "Facer.hpp"
#include "Follower.hpp"
#include "Stalker.hpp"
#include "Agent.hpp"

#include "Player.hpp"

#include "Items.hpp"

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
static GObject::AdapterType consAdapter()
{
    return [](const ValueMap& args) -> GObject* { return new T(args); };
}

//Inventory adaptor: will return nullptr if the item has already been acquired,
//meaning item will not be added.
template <typename T>
static GObject::AdapterType itemAdapter(const string& name)
{
    return [=](const ValueMap& args) -> GObject* {
        if(GState::crntState.itemRegistry.find(name) != GState::crntState.itemRegistry.end())
            return nullptr;
        else return new T(args);
    };
}

#define entry(name,cls) (name, consAdapter<cls>())
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define item_entry(name,cls,itemKey) (name, itemAdapter<cls>(#itemKey))
#define item_entry_same(cls) item_entry(#cls,cls,cls)

const unordered_map<string,GObject::AdapterType> GObject::adapters =
    boost::assign::map_list_of
    entry_same(Block)
    entry_same(FireBullet)
    entry_same(WaterBullet)
    entry_same(StarBullet)
    entry_same(Flower)
    entry_same(Glyph)
    entry_same(Torch)
    entry_same(CollectGlyph)

    item_entry_same(GraveyardKey)

    entry_same(Marisa)
    entry_same(Patchouli)
    entry_same(Reimu)

    entry_same(Facer)
    entry_same(Follower)
    entry_same(Stalker)
    entry_same(Agent)

    entry_same(Player);
