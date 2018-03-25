//
//  AIMixins.cpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#include "Prefix.h"
#include "AIMixins.hpp"

StateMachineObject::StateMachineObject(shared_ptr<ai::State> startState, const ValueMap& args) :
    RegisterUpdate(this),
    fsm(this)
{
    shared_ptr<ai::State> stateFromArgs = nullptr;
    auto searchIter = args.find("startState");
    
    if(searchIter != args.end() && searchIter->second.asString() == "none")
        return;

    //If startState is provided in the object args map, try to construct it.
    if(searchIter != args.end())
    {
        stateFromArgs = ai::State::constructState(searchIter->second.asString(), args);
        
        if(!stateFromArgs){
            log("%s: failed to construct state %s", getName().c_str(),searchIter->second.asString().c_str());
        }
    }
    
    //If stateFromArgs was constructed, it will override the provided state.
    if(stateFromArgs)
        fsm.push(stateFromArgs);
    else if(startState)
        fsm.push(startState);
}

void RadarObject::update()
{
    updateRadarPos();
    updateVisibleObjects();
}

//Objects that gain/lose visibility because they enter/exit sensor range will be
//processed during the next update.
void RadarObject::radarCollision(GObject* other)
{
    objectsToAdd.push_back(other);
}
void RadarObject::radarEndCollision(GObject* other)
{
    objectsToRemove.push_back(other);
}

bool RadarObject::isObjectVisible(GObject* other)
{
    if(fovAngle == 0.0f) return true;

	SpaceVect facingUnit = SpaceVect::ray(1.0f, getAngle());
    SpaceVect displacementUnit = (other->getPos() - getPos()).normalize();
    float scalar = SpaceVect::dot(facingUnit,displacementUnit);
    
    return scalar >= fovScalar;
}

void RadarObject::setFovAngle(float angle)
{
    if(angle == 0.0f){
        fovAngle = 0.0f;
        fovScalar = 0.0f;
    } else {
        fovScalar = cos(angle);
        fovAngle = angle;
    }
}

void RadarObject::updateVisibleObjects()
{
    BOOST_FOREACH(GObject* obj, objectsInRange)
    {
        bool currentlyVisible = isObjectVisible(obj);
        bool previouslyVisible = visibleObjects.find(obj) != visibleObjects.end();
        
        if(currentlyVisible && !previouslyVisible){
            onDetect(obj);
            visibleObjects.insert(obj);
        }
        else if(!currentlyVisible && previouslyVisible){
            onEndDetect(obj);
            visibleObjects.erase(obj);
        }
    }
    
    BOOST_FOREACH(GObject* obj, objectsToAdd)
    {
        objectsInRange.insert(obj);
        
        if(isObjectVisible(obj)){
            onDetect(obj);
            visibleObjects.insert(obj);
        }
    }
    objectsToAdd.clear();
    
    BOOST_FOREACH(GObject* obj, objectsToRemove)
    {
        objectsInRange.erase(obj);
        
        if(isObjectVisible(obj)){
            onEndDetect(obj);
            visibleObjects.erase(obj);
        }
    }
    objectsToRemove.clear();
}

GObject* RadarObject::getSensedObject()
{
	SpaceVect facingUnit = SpaceVect::ray(1, getAngle());

	float bestScalar = -1;
	GObject* bestObj = nullptr;

	BOOST_FOREACH(GObject* obj, visibleObjects)
	{
		SpaceVect displacementUnit = (obj->getPos() - getPos()).normalize();

		float dot = SpaceVect::dot(facingUnit, displacementUnit);

		if (dot > bestScalar)
		{
			bestScalar = dot;
			bestObj = obj;
		}
	}

	return bestObj;
}
