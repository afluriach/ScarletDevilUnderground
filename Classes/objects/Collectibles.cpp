//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Collectibles.hpp"

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos) :
GObject(space,id,"spriteName",pos,true)
{
}

const AttributeMap Power1::effect = {
	{Attribute::power, 5.0f}
};

const string Power1::spriteName = "power1";

Power1::Power1(GSpace* space, ObjectIDType id, SpaceVect pos):
GObject(space, id, "Power1", pos, true),
CollectibleImpl<Power1>(space,id,pos)
{
}

const AttributeMap Power2::effect = {
	{ Attribute::power, 25.0f }
};

const string Power2::spriteName = "power2";


Power2::Power2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "Power2", pos, true),
	CollectibleImpl<Power2>(space, id, pos)
{
}

const AttributeMap Magic1::effect = {
	{ Attribute::mp, 0.25f }
};

const string Magic1::spriteName = "magic1";

Magic1::Magic1(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "Magic1", pos, true),
	CollectibleImpl<Magic1>(space, id, pos)
{
}

const AttributeMap Magic2::effect = {
	{ Attribute::mp, 1.0f }
};

const string Magic2::spriteName = "magic2";

Magic2::Magic2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "Magic2", pos, true),
	CollectibleImpl<Magic2>(space, id, pos)
{
}

