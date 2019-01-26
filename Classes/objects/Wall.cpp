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

Wall::Wall(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space,id,args,true)
{}

Wall::Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions) :
	GObject(space, id, "wall", center, true),
	RectangleBody(dimensions)
{}

PhysicsLayers Wall::getLayers() const{
    return PhysicsLayers::all;
}
