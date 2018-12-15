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
        stateFromArgs = ai::Function::constructState(searchIter->second.asString(), space, args);
        
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
        0.1,
        getRadarType(),
        PhysicsLayers::all,
        true,
        this
    );
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
	if (other->getInvisible())
		return false;

    if(fovAngle == 0.0)
        return ai::isLineOfSight(*this, *other);

	SpaceVect facingUnit = SpaceVect::ray(1.0, getAngle());
    SpaceVect displacementUnit = (other->getPos() - getPos()).normalize();
    SpaceFloat scalar = SpaceVect::dot(facingUnit,displacementUnit);
    
    return scalar >= fovScalar && ai::isLineOfSight(*this, *other);
}

void RadarObject::setFovAngle(SpaceFloat angle)
{
    if(angle == 0.0){
        fovAngle = 0.0;
        fovScalar = 0.0;
    } else {
        fovScalar = cos(angle);
        fovAngle = angle;
    }
}

void RadarObject::update()
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

	SpaceFloat bestScalar = -1.0;
	GObject* bestObj = nullptr;

	BOOST_FOREACH(GObject* obj, visibleObjects)
	{
		SpaceVect displacementUnit = (obj->getPos() - getPos()).normalize();

		SpaceFloat dot = SpaceVect::dot(facingUnit, displacementUnit);

		if (dot > bestScalar)
		{
			bestScalar = dot;
			bestObj = obj;
		}
	}

	return bestObj;
}
