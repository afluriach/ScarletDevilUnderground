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
#include "Door.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnvironmentObject.hpp"
#include "FloorSegment.hpp"
#include "Item.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "Torch.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

make_static_member_detector(properName)

//Adapters for mapping the name of a class to a factory adapter.
template <typename T>
GObject::AdapterType consAdapter()
{
    return [](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		return allocator_new<T>(space,id,args);
	};
}

GObject::AdapterType itemAdapter()
{
    return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		object_params params(args);
		string typeName = getStringOrDefault(args, "item", "");
		auto itemProps = app::getItem(typeName);

		if (!itemProps) {
			log("Unknown Item type: %s", typeName);
		}

		if (!itemProps || !Item::conditionalLoad(space, params, itemProps))
			return nullptr;
        else
			return allocator_new<Item>(space, id, params, itemProps);
    };
}

template<typename T>
GObject::AdapterType conditionalLoadAdapter()
{
	return [=](GSpace* space, ObjectIDType id, const ValueMap& args) -> GObject* {
		if (!T::conditionalLoad(space, id, args) )
			return nullptr;
		else return allocator_new<T>(space, id, args);
	};
}

template <class C>
GObject::object_info makeObjectInfo(GObject::AdapterType adapter)
{
	return GObject::object_info{
		adapter,
		type_index(typeid(C))
	};
}

#define entry(name,cls) {name, makeObjectInfo<cls>(consAdapter<cls>())}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

#define no_adapter_entry(name) {#name, makeObjectInfo<name>(nullptr)}
#define conditional_entry(name) {#name, makeObjectInfo<name>(conditionalLoadAdapter<name>())}

unordered_map<string, GObject::object_info> GObject::objectInfo;
unordered_map<string, GObject::AdapterType> GObject::namedObjectTypes;

void GObject::initObjectInfo()
{
	objectInfo = {

	no_adapter_entry(AreaSensor),
	no_adapter_entry(AreaSensorImpl),
	no_adapter_entry(Bullet),
	entry_same(Door),
	no_adapter_entry(EffectArea),
	no_adapter_entry(Enemy),
	no_adapter_entry(EnvironmentObject),
	no_adapter_entry(FloorSegment),
	entry_same(HiddenSubroomSensor),
	{"Item", makeObjectInfo<Item>(itemAdapter())},
	no_adapter_entry(NPC),
	entry_same(Pitfall),
	no_adapter_entry(Player),
	no_adapter_entry(RoomSensor),
	entry_same(Torch),
	entry_same(Wall),

	};

}

const unordered_set<type_index> GObject::trackedTypes = {
	typeid(Bullet),
	typeid(Door),
	typeid(Enemy),
	typeid(FloorSegment),
	typeid(Item),
	typeid(Pitfall),
	typeid(Player),
	typeid(RoomSensor),
	typeid(Wall),
};

#define _nameTypeEntry(cls) {#cls, typeid(cls)}
