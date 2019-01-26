//
//  Glyph.cpp
//  Koumachika
//
//  Created by Toni on 10/28/15.
//
//

#include "Prefix.h"

#include "Glyph.hpp"

Glyph::Glyph(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RegisterInit<Glyph>(this)
{}

void Glyph::init()
{
    setVel(SpaceVect(0,-6));
}
