//
//  Collectibles.cpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#include "Prefix.h"

#include "Collectibles.hpp"

Collectible::Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, const string& spriteName, AttributeMap effect) :
GObject(space,id,"spriteName",pos,true),
spriteName(spriteName),
effect(effect)
{
}

const AttributeMap Power1::effect = {
	{Attribute::power, 5.0f}
};

Power1::Power1(GSpace* space, ObjectIDType id, SpaceVect pos):
GObject(space, id, "Power1", pos, true),
Collectible(space,id,pos,"power1",effect)
{
}

const AttributeMap Power2::effect = {
	{ Attribute::power, 25.0f }
};

Power2::Power2(GSpace* space, ObjectIDType id, SpaceVect pos) :
	GObject(space, id, "Power2", pos, true),
	Collectible(space, id, pos, "power2", effect)
{
}
