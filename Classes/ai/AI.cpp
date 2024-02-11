//
//  AI.cpp
//  Koumachika
//
//  Created by Toni on 12/13/15.
//
//

#include "Prefix.h"

#include "AI.hpp"

namespace ai{

Function::Function(GObject* object) :
	object(object)
{}

Function::~Function()
{
}

GSpace* Function::getSpace() const {
	return object->space;
}

GObject* Function::getObject() const {
	return object;
}

physics_context* Function::getPhys() const {
	return object->space->physicsContext.get();
}

}//end NS
