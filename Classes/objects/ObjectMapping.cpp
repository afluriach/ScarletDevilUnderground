//
//  ObjectMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AreaSensor.hpp"
#include "Bullet.hpp"
#include "CollectGlyph.hpp"
#include "Collectibles.hpp"
#include "Door.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnvironmentalObjects.hpp"
#include "FloorSegment.hpp"
#include "Item.hpp"
#include "Player.hpp"
#include "Spawner.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"
#include "value_map.hpp"
#include "Wall.hpp"
#include "xml.hpp"

make_static_member_detector(properName)

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
constexpr GObject::AdapterType consAdapter()
{
    return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* { return new T(space,id,args); };
}

GObject::AdapterType itemAdapter()
{
    return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		item_attributes attr = Item::parseAttributes(args);
		string typeName = getStringOrDefault(args, "item", "");
		auto itemProps = app::getItem(typeName);

		if (!itemProps) {
			log("Unknown Item type: %s", typeName);
		}

		if (!itemProps || !Item::conditionalLoad(space, attr, itemProps))
			return nullptr;
        else
			return new Item(space,id,attr, itemProps);
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

GObject::AdapterType collectibleAdapter(string coll_id)
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

#define entry(name,cls) {name, makeObjectInfo<cls>(consAdapter<cls>())}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define conditional_entry(name) {#name, makeObjectInfo<name>(conditionalLoadAdapter<name>())}

unordered_map<string, GObject::object_info> GObject::objectInfo;
unordered_map<string, GObject::AdapterType> GObject::namedObjectTypes;

void GObject::initObjectInfo()
{
	objectInfo = {

	entry_same(CollectGlyph),
	entry_same(DarknessArea),
	entry_same(Door),
	entry_same(GhostHeadstone),
	conditional_entry(Headstone),
	entry_same(HiddenSubroomSensor),
	entry_same(IcePlatform),
	{"Item", makeObjectInfo<Item>(itemAdapter())},
	entry_same(MovingPlatform),
	entry_same(Pitfall),
	entry_same(PressurePlate),
	entry_same(Spawner),
	entry_same(SunArea),
	entry_same(TeleportPad),
	entry_same(Torch),
	entry_same(Wall),

	};

#define collectible_entry(name,id) {#name, collectibleAdapter(#id)}
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
	typeid(Bullet),
	typeid(Door),
	typeid(Enemy),
	typeid(FloorSegment),
	typeid(Item),
	typeid(Pitfall),
	typeid(Player),
	typeid(RoomSensor),
	typeid(Spawner),
	typeid(TeleportPad),
	typeid(Wall),
};

#define _nameTypeEntry(cls) {#cls, typeid(cls)}
