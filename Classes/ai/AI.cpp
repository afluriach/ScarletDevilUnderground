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

bool Function::isActive() const {
    return _state == state::active;
}

bool Function::isCompleted() const{
    return _state == state::completed;
}

void Function::runEnter()
{
    if(_state != state::created){
        log1("%s: Invalid call to runEnter!", getName());
        return;
    }
    
    onEnter();
    _state = state::active;
}

void Function::runUpdate()
{
    if(_state == state::created){
        runEnter();
        _state = state::active;
    }

    if(_state != state::active){
        log1("%s: Invalid call to runUpdate!", getName());
        return;
    }
    
    update();
}

void Function::runExit()
{
    if(_state != state::active && _state != state::completing){
        log1("%s: Invalid call to runExit!", getName());
        return;
    }
    
    onExit();
    _state = state::completed;
}

}//end NS
