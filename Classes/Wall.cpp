//
//  Wall.cpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#include "Prefix.h"

#include "macros.h"
#include "Wall.hpp"

PhysicsLayers Wall::getLayers() const{
    return enum_bitwise_or(PhysicsLayers,ground,eyeLevel);
}
