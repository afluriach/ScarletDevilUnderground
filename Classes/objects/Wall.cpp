//
//  Wall.cpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#include "Prefix.h"

#include "enum.h"
#include "Wall.hpp"

PhysicsLayers Wall::getLayers() const{
    return PhysicsLayers::all;
}
