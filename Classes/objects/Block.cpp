//
//  Block.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "Block.hpp"
#include "enum.h"
#include "value_map.hpp"

Block::Block(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjParams(),
MassImpl(getFloatOrDefault(args,"mass",40.0))
{
	setInitialAngle(float_pi / 2.0);
}

PhysicsLayers Block::getLayers() const{
    return enum_bitwise_or3(PhysicsLayers,floor,ground,eyeLevel);
}
