//
//  AIMixins.cpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#include "Prefix.h"

#include "AIMixins.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"

StateMachineObject::StateMachineObject(shared_ptr<ai::Function> startState, const ValueMap& args) :
    RegisterUpdate(this),
    fsm(this)
{
    shared_ptr<ai::Function> stateFromArgs = nullptr;
    auto searchIter = args.find("startState");
    
    if(searchIter != args.end() && searchIter->second.asString() == "none")
        return;

    //If startState is provided in the object args map, try to construct it.
    if(searchIter != args.end())
    {
        stateFromArgs = ai::Function::constructState(searchIter->second.asString(), args);
        
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

    void RadarObject::onDetect(GObject* other) {
		ctx->callIfExistsNoReturn("onDetect", ctx->makeArgs(other));
	}

	void RadarObject::onEndDetect(GObject* other) {
		ctx->callIfExistsNoReturn("onEndDetect", ctx->makeArgs(other));
	}


void RadarObject::initializeRadar(GSpace& space)
{
    radar = space.createCircleBody(
        initialCenter,
        getRadarRadius(),
        0.1f,
        getRadarType(),
        PhysicsLayers::all,
        true,
        this
    );
}

void RadarObject::update()
{
    updateRadarPos();
    updateVisibleObjects();
}

//Objects that gain/lose visibility because they enter/exit sensor range will be
//processed during the next update.
void RadarObject::radarCollision(GObject* obj)
{
	objectsInRange.insert(obj);

	if (isObjectVisible(obj)) {
		onDetect(obj);
		visibleObjects.insert(obj);
	}

}
void RadarObject::radarEndCollision(GObject* obj)
{
	objectsInRange.erase(obj);

	if (isObjectVisible(obj)) {
		onEndDetect(obj);
		visibleObjects.erase(obj);
	}
}

bool RadarObject::isObjectVisible(GObject* other)
{
    if(fovAngle == 0.0f)
        return ai::isLineOfSight(*this, *other);

	SpaceVect facingUnit = SpaceVect::ray(1.0f, getAngle());
    SpaceVect displacementUnit = (other->getPos() - getPos()).normalize();
    float scalar = SpaceVect::dot(facingUnit,displacementUnit);
    
    return scalar >= fovScalar && ai::isLineOfSight(*this, *other);
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
