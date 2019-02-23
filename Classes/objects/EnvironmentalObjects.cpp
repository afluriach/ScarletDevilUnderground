//
//  EnvironmentalObjects.cpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#include "Prefix.h"

#include "EnvironmentalObjects.hpp"
#include "macros.h"

Headstone::Headstone(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args),
RectangleBody(args)
{
	setInitialDirectionOrDefault(args, Direction::up);
}

Sapling::Sapling(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
	setInitialAngle(float_pi / 2.0);
}
