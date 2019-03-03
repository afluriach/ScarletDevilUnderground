//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Collectibles.hpp"
#include "value_map.hpp"

template<class C>
function<ObjectGeneratorType(SpaceVect) > createAdapter()
{
	return [](SpaceVect& pos) -> ObjectGeneratorType {
		return GObject::make_object_factory<C>(pos);
	};
}

const map<collectible_id, function<ObjectGeneratorType(SpaceVect)>> Collectible::factories = {
	{collectible_id::magic1, createAdapter<Magic1>() },
	{collectible_id::magic2, createAdapter<Magic2>() },
	{collectible_id::power1, createAdapter<Power1>() },
	{collectible_id::power2, createAdapter<Power2>() },
	{collectible_id::health1, createAdapter<Health1>() },
	{collectible_id::health2, createAdapter<Health2>() },
	{collectible_id::key, createAdapter<Key>() },
};

ObjectGeneratorType Collectible::create(collectible_id id, SpaceVect pos)
{
	return factories.at(id)(pos);
}

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos) :
GObject(space,id,"",pos, float_pi / 2.0),
RectangleBody(SpaceVect(0.5, 0.5))
{
}

const AttributeMap Power1::effect = {
	{Attribute::power, 5.0f}
};

const string Power1::spriteName = "power1";

Power1::Power1(GSpace* space, ObjectIDType id, SpaceVect pos):
GObject(space, id, "", pos, float_pi / 2.0),
CollectibleImpl<Power1>(space,id,pos)
{
}

const AttributeMap Power2::effect = {
	{ Attribute::power, 25.0f }
};

const string Power2::spriteName = "power2";


Power2::Power2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Power2>(space, id, pos)
{
}

const AttributeMap Magic1::effect = {
	{ Attribute::mp, 0.25f }
};

const string Magic1::spriteName = "magic1";

Magic1::Magic1(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "", pos, float_pi / 2.0),
	CollectibleImpl<Magic1>(space, id, pos)
{
}

const AttributeMap Magic2::effect = {
	{ Attribute::mp, 1.0f }
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
