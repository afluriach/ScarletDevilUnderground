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
#include "AreaSensor.hpp"
#include "Block.hpp"
#include "Bullet.hpp"
#include "CollectGlyph.hpp"
#include "Collectibles.hpp"
#include "Desk.hpp"
#include "Door.hpp"
#include "EffectArea.hpp"
#include "EnvironmentalObjects.hpp"
#include "Fairy.hpp"
#include "FairyNPC.hpp"
#include "FloorSegment.hpp"
#include "Flower.h"
#include "Goal.hpp"
#include "Glyph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Items.hpp"
#include "Launcher.hpp"
#include "MapFragment.hpp"
#include "Marisa.hpp"
#include "Meiling.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Pyramid.hpp"
#include "Reimu.hpp"
#include "Rumia.hpp"
#include "SakuyaNPC.hpp"
#include "Sign.hpp"
#include "Spawner.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"
#include "Upgrade.hpp"
#include "Wall.hpp"
#include "xml.hpp"

make_static_member_detector(properName)

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
        if(space->getState()->hasItem(name))
            return nullptr;
        else return new T(space,id,args);
    };
}

template<typename T>
GObject::AdapterType conditionalLoadAdapter()
{
	return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		if (!T::conditionalLoad(space, id, args) )
			return nullptr;
		else return new T(space, id, args);
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

GObject::AdapterType collectibleAdapter(collectible_id coll_id)
{
	return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		SpaceVect pos = getObjectPos(args);
		return new Collectible(space, id, pos, coll_id);
	};
}

template <class C>
GObject::object_info makeObjectInfo(GObject::AdapterType adapter)
{
	string name;
	if constexpr(has_properName<C>::value) {
		name = C::properName;
	}

	return GObject::object_info{
		adapter,
		type_index(typeid(C)),
		name
	};
}

GObject::object_info playerObjectInfo()
{
	return makeObjectInfo<Player>(playerAdapter());
}

#define entry(name,cls) {name, makeObjectInfo<cls>(consAdapter<cls>())}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define conditional_entry(name) {#name, makeObjectInfo<name>(conditionalLoadAdapter<name>())}

#define item_entry(name,cls,itemKey) {name, makeObjectInfo<cls>(itemAdapter<cls>(#itemKey))}
#define item_entry_same(cls) item_entry(#cls,cls,cls)

unordered_map<string, GObject::object_info> GObject::objectInfo;
unordered_map<string, GObject::AdapterType> GObject::namedObjectTypes;

void GObject::initObjectInfo()
{
	objectInfo = {

	conditional_entry(BlueFairyNPC),
	entry_same(BreakableWall),
	entry_same(CollectGlyph),
	entry_same(Barrier),
	entry_same(Block),
	entry_same(DarknessArea),
	entry_same(Desk),
	entry_same(Door),
	entry_same(Fairy2),
	entry_same(FairyMaid),
	entry_same(Flower),
	item_entry_same(ForestBook1),
	entry_same(GenericAgent),
	conditional_entry(GhostFairyNPC),
	entry_same(GhostHeadstone),
	entry_same(GhostHeadstoneSensor),
	entry_same(Glyph),
	entry_same(Goal),
	item_entry_same(GraveyardBook1),
	conditional_entry(Headstone),
	entry_same(HiddenSubroomSensor),
	entry_same(IcePlatform),
	entry_same(Launcher),
	conditional_entry(MapFragment),
	entry_same(MarisaNPC),
	conditional_entry(Meiling1),
	entry_same(MovingPlatform),
	conditional_entry(Mushroom),
	entry_same(Patchouli),
	entry_same(PatchouliEnemy),
	entry_same(Pitfall),
	entry_same(PressurePlate),
	entry_same(Pyramid),
	entry_same(RedFairy),
	entry_same(Reimu),
	entry_same(ReimuEnemy),
	entry_same(Rumia1),
	entry_same(Rumia2),
	entry_same(SakuyaNPC),
	entry_same(Sapling),
	entry_same(Sign),
	entry_same(Spawner),
	conditional_entry(Spellcard),
	entry_same(SunArea),
	entry_same(TeleportPad),
	entry_same(Torch),
	conditional_entry(Upgrade),
	entry_same(Wall),
	entry_same(WaterFloor),

	{ "Player", playerObjectInfo() }

	};

#define collectible_entry(name,id) {#name, collectibleAdapter(collectible_id::id)}
#define enemy_entry(name,id) {#name, enemyAdapter(#id)}

	namedObjectTypes = {
	
	collectible_entry(Health1, health1),
	collectible_entry(Health2, health2),
	collectible_entry(Health3, health3),
	collectible_entry(Key, key),
	collectible_entry(Magic1, magic1),
	collectible_entry(Magic2, magic2),
	collectible_entry(Magic3, magic3),

	};

}

const unordered_set<type_index> GSpace::trackedTypes = {
	typeid(Door),
	typeid(RoomSensor),
	typeid(Spawner),
	typeid(TeleportPad),

	//virtual tracked types
	typeid(Player),
	typeid(Enemy),
	typeid(Bullet),
	typeid(FloorSegment),
	typeid(Wall),
};

const unordered_set<type_index> GSpace::enemyTypes = {
	typeid(Fairy2),

	typeid(RedFairy),
	
	typeid(Rumia1),
	typeid(Rumia2),
	typeid(PatchouliEnemy),
	typeid(ReimuEnemy),
};

#define _nameTypeEntry(cls) {#cls, typeid(cls)}
