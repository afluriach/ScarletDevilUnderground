//
//  ObjectMapping.cpp
//  Koumachika
//
//  Created by Toni on 11/22/15.
//
//

#include "Prefix.h"

#include "Bomb.hpp"
#include "Bullet.hpp"
#include "Door.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnvironmentObject.hpp"
#include "FloorSegment.hpp"
#include "Item.hpp"
#include "NPC.hpp"
#include "Player.hpp"
#include "Wall.hpp"

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

typedef function<GObject*(GSpace*, ObjectIDType, const object_params& params, local_shared_ptr<object_properties> props)> PropsAdapter;

template<typename T, typename P>
PropsAdapter objectAdapter()
{
	return [](
		GSpace* space,
		ObjectIDType id,
		const object_params& params,
		local_shared_ptr<object_properties> props
	) -> GObject* {
		auto p = props.downcast<P>();
		return allocator_new<T>(space, id, params, p);
	};
}

#define entry(prop,obj) { typeid(prop), objectAdapter<obj,prop>() }

const unordered_map<type_index, PropsAdapter> GObject::propsAdapters = {
	entry(bomb_properties, Bomb),
	entry(effectarea_properties, EffectArea),
	entry(enemy_properties, Enemy),
	entry(environment_object_properties, EnvironmentObject),
	entry(item_properties, Item),
	entry(npc_properties, NPC)
};
