//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Collectibles.hpp"
#include "GSpace.hpp"
#include "value_map.hpp"

template<class C>
function<ObjectGeneratorType(GSpace*, SpaceVect) > createAdapter()
{
	return [](GSpace* space, SpaceVect pos) -> ObjectGeneratorType {
		return GObject::make_object_factory<C>(pos);
	};
}

template<class C1, class C2>
function<ObjectGeneratorType(GSpace* space, SpaceVect) > createRandomAdapter()
{
	return [](GSpace* space, SpaceVect pos) -> ObjectGeneratorType {
		if (space->getRandomFloat() < 0.5f)
			return GObject::make_object_factory<C1>(pos);
		else
			return GObject::make_object_factory<C2>(pos);
	};
}

const unordered_map<collectible_id, function<ObjectGeneratorType(GSpace*, SpaceVect)>> Collectible::factories = {
	{collectible_id::magic1, createAdapter<Magic1>() },
	{collectible_id::magic2, createAdapter<Magic2>() },
	{collectible_id::health1, createAdapter<Health1>() },
	{collectible_id::health2, createAdapter<Health2>() },
	{collectible_id::hm1, createRandomAdapter<Health1, Magic1>() },
	{collectible_id::hm2, createRandomAdapter<Health2, Magic2>() },
	{collectible_id::key, createAdapter<Key>() },
};

ObjectGeneratorType Collectible::create(GSpace* space, collectible_id id, SpaceVect pos)
{
	return factories.at(id)(space, pos);
}

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos) :
GObject(space,id,"",pos, float_pi / 2.0),
RectangleBody(SpaceVect(0.5, 0.5))
{
}

const AttributeMap Magic1::effect = {
	{ Attribute::mp, 1.0f }
};

const string Magic1::spriteName = "magic1";

Magic1::Magic1(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Magic1>(space, id, pos)
{
}

const AttributeMap Magic2::effect = {
	{ Attribute::mp, 5.0f }
};

const string Magic2::spriteName = "magic2";

Magic2::Magic2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Magic2>(space, id, pos)
{
}

const AttributeMap Health1::effect = {
	{ Attribute::hp, 5.0f }
};

const string Health1::spriteName = "health1";

Health1::Health1(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Health1>(space, id, pos)
{
}

const AttributeMap Health2::effect = {
	{ Attribute::hp, 25.0f }
};

const string Health2::spriteName = "health2";

Health2::Health2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Health2>(space, id, pos)
{
}

const AttributeMap Key::effect = {
	{Attribute::keys, 1.0f}
};

const string Key::spriteName = "key";

Key::Key(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space,id,"", pos, float_pi / 2.0),
	CollectibleImpl<Key>(space,id,pos)
{

}
