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
#include "EffectArea.hpp"
#include "Facer.hpp"
#include "Fairy.hpp"
#include "Follower.hpp"
#include "FloorSegment.hpp"
#include "Flower.h"
#include "Goal.hpp"
#include "Glyph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Launcher.hpp"
#include "Marisa.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Pyramid.hpp"
#include "Reimu.hpp"
#include "Sakuya.hpp"
#include "Stalker.hpp"
#include "Tewi.hpp"
#include "Torch.hpp"
#include "Wall.hpp"

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
constexpr GObject::AdapterType consAdapter()
{
    return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* { return new T(space,id,args); };
}

//Inventory adaptor: will return nullptr if the item has already been acquired,
//meaning item will not be added.
template <typename T>
constexpr GObject::AdapterType itemAdapter(const string& name)
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
		switch (App::crntPC) {
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

#define entry(name,cls) {name, consAdapter<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define item_entry(name,cls,itemKey) {name, itemAdapter<cls>(#itemKey)}
#define item_entry_same(cls) item_entry(#cls,cls,cls)

const unordered_map<string, GObject::AdapterType> GObject::adapters = {
	entry_same(CollectGlyph),
	entry_same(CollectMarisa),
	entry_same(Block),
	entry_same(DirtFloorCave),
	entry_same(Facer),
	entry_same(Fairy1),
	entry_same(Fairy2),
	entry_same(Flower),
	entry_same(Follower),
	entry_same(ForestMarisa),
	entry_same(GenericAgent),
	entry_same(Glyph),
	entry_same(Goal),
	item_entry_same(GraveyardKey),
	entry_same(IceFairy),
	entry_same(IceFloor),
	entry_same(IcePlatform),
	entry_same(Launcher),
	entry_same(MineFloor),
	entry_same(MovingPlatform),
	entry_same(Patchouli),
	entry_same(Pitfall),
	entry_same(PressurePlate),
	entry_same(Pyramid),
	entry_same(Reimu),
	entry_same(Sakuya),
	entry_same(Stalker),
	entry_same(SunArea),
	entry_same(Tewi),
	entry_same(Torch),
	entry_same(Wall),

	{ "Player", playerAdapter() }
};

const set<type_index> GSpace::trackedTypes = {
	typeid(Fairy2)
};
