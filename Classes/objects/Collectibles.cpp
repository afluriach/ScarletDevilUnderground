//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Attributes.hpp"
#include "Collectibles.hpp"
#include "GSpace.hpp"
#include "value_map.hpp"

#define entry(x) (collectible_id::x, #x)

const boost::bimap<collectible_id, string> Collectible::collectibleNameMap = boost::assign::list_of<boost::bimap<collectible_id, string>::relation>()
	entry(health1)
	entry(health2)
	entry(health3)

	entry(magic1)
	entry(magic2)
	entry(magic3)

	entry(hm1)
	entry(hm2)
	entry(hm3)

	entry(key)
;

#undef entry

#define entry(x, spr, attr, val) { collectible_id::x, {spr, Attribute::attr, val} }

const unordered_map<collectible_id, collectible_properties> Collectible::propertiesMap = {
	entry(health1, "health1", hp, 5.0f),
	entry(health2, "health2", hp, 25.0f),
	entry(health3, "health3", hp, 100.0f),
	entry(key, "key", keys, 1.0f),
	entry(magic1, "magic1", mp, 5.0f),
	entry(magic2, "magic2", mp, 25.0f),
	entry(magic3, "magic3", mp, 100.0f),
};

#undef entry

function<collectible_id(GSpace* space) > createRandomAdapter(collectible_id c1, collectible_id c2)
{
	return [c1,c2](GSpace* space) -> collectible_id {
		return space->getRandomFloat() < 0.5f ? c1 : c2;
	};
}

const unordered_map<collectible_id, function<collectible_id(GSpace*)>> Collectible::vMap = {
	{collectible_id::hm1, createRandomAdapter(collectible_id::health1, collectible_id::magic1) },
	{collectible_id::hm2, createRandomAdapter(collectible_id::health2, collectible_id::magic2) },
	{collectible_id::hm3, createRandomAdapter(collectible_id::health3, collectible_id::magic3) },
};

ObjectGeneratorType Collectible::create(GSpace* space, collectible_id id, SpaceVect pos)
{
	if (id == collectible_id::nil)
		return GObject::null_object_factory();

	collectible_id actualID = id;
	auto it = vMap.find(id);
	if (it != vMap.end()) {
		actualID = it->second(space);
	}

	return GObject::make_object_factory<Collectible>(pos, actualID);
}

collectible_id Collectible::getCollectibleID(const string& name)
{
	auto it = collectibleNameMap.right.find(name);

	if (it == collectibleNameMap.right.end()) {
		log("Unknown collectible_id name %s", name);
	}

	return it != collectibleNameMap.right.end() ? it->second : collectible_id::nil;
}

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, collectible_id collectibleID) :
	GObject(make_shared<object_params>(space, id, "", pos)),
	RectangleBody(SpaceVect(0.5, 0.5)),
	collectibleID(collectibleID)
{
}

string Collectible::getSprite() const
{
	auto it = propertiesMap.find(collectibleID);
	return it->second.sprite;
}

AttributeMap Collectible::getEffect() const
{
	auto it = propertiesMap.find(collectibleID);
	return { {it->second.attr, it->second.val} };
}
