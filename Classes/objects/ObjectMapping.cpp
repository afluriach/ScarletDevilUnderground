//
//  ObjectMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "Block.hpp"
#include "Bullet.hpp"
#include "CollectGlyph.hpp"
#include "Collectibles.hpp"
#include "Facer.hpp"
#include "Fairy.hpp"
#include "Follower.hpp"
#include "FloorSegment.hpp"
#include "Flower.h"
#include "Goal.hpp"
#include "Glyph.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Marisa.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Reimu.hpp"
#include "Sakuya.hpp"
#include "Stalker.hpp"
#include "Tewi.hpp"
#include "Torch.hpp"

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
static GObject::AdapterType consAdapter()
{
    return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* { return new T(space,id,args); };
}

//Inventory adaptor: will return nullptr if the item has already been acquired,
//meaning item will not be added.
template <typename T>
static GObject::AdapterType itemAdapter(const string& name)
{
    return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
        if(GState::crntState.itemRegistry.find(name) != GState::crntState.itemRegistry.end())
            return nullptr;
        else return new T(space,id,args);
    };
}

GObject::AdapterType playerAdapter()
{
	return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		switch (app->crntPC) {
		case PlayerCharacter::flandre:
			return new FlandrePC(space,id,args);
		case PlayerCharacter::rumia:
			return new RumiaPC(space,id,args);
		case PlayerCharacter::cirno:
			return new CirnoPC(space,id,args);
		default:
			return nullptr;
		}
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
    entry_same(Flower)
    entry_same(Glyph)
    entry_same(Torch)
    entry_same(CollectGlyph)

    entry_same(PowerUp)

    item_entry_same(GraveyardKey)

    entry_same(Marisa)
    entry_same(Patchouli)
    entry_same(Reimu)
    entry_same(Sakuya)

    entry_same(Facer)
    entry_same(Follower)
    entry_same(Stalker)
    entry_same(GenericAgent)
	entry_same(Tewi)

	entry_same(Fairy1)
	entry_same(Fairy2)
	entry_same(IceFairy)

	entry_same(DirtFloorCave)
	entry_same(Pitfall)

	entry_same(Goal)

	("Player", playerAdapter());
