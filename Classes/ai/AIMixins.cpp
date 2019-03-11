//
//  AIMixins.cpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "AIMixins.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"

StateMachineObject::StateMachineObject() :
	fsm(this)
{
}

StateMachineObject::StateMachineObject(shared_ptr<ai::Function> startState, const ValueMap& args) :
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

StateMachineObject::StateMachineObject(const ValueMap& args) : StateMachineObject(nullptr, args) {}

void StateMachineObject::_update() {
	if (!isFrozen)
		fsm.update();
}

unsigned int StateMachineObject::addThread(shared_ptr<ai::Function> threadMain) {
	return fsm.addThread(threadMain);
}

void StateMachineObject::removeThread(unsigned int uuid) {
	fsm.removeThread(uuid);
}

void StateMachineObject::removeThread(const string& name) {
	fsm.removeThread(name);
}

void StateMachineObject::printFSM() {
	log("%s", fsm.toString().c_str());
}

void StateMachineObject::setFrozen(bool val) {
	isFrozen = val;
}

RadarObject::RadarObject() :
	RegisterInit(this)
{}

void RadarObject::init() {
	setFovAngle(getDefaultFovAngle());
}

void RadarObject::initializeRadar(GSpace& space)
{
    tie(radarShape,radar) = space.createCircleBody(
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
	if (this == obj)
		return;

	objectsInRange.insert(obj);

	if (isObjectVisible(obj)) {
		onDetect(obj);
		visibleObjects.insert(obj);
	}

}
void RadarObject::radarEndCollision(GObject* obj)
{
	if (this == obj)
		return;

	objectsInRange.erase(obj);

	if (visibleObjects.find(obj) != visibleObjects.end()) {
		onEndDetect(obj);
		visibleObjects.erase(obj);
	}
}

bool RadarObject::isObjectVisible(GObject* other)
{
	if (other->getInvisible())
		return false;

	bool isFov = fovAngle == 0.0 ? true : ai::isInFieldOfView(this, other->getPos(), fovScalar);
	bool isLos = hasEssenceRadar() ? true : ai::isLineOfSight(this, other);

	return isFov && isLos;
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

void RadarObject::_update()
{
    for(GObject* obj: objectsInRange)
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

	for(GObject* obj: visibleObjects)
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

list<GObject*> RadarObject::getSensedObjects()
{
	return list<GObject*>(visibleObjects.begin(), visibleObjects.end());
}

list<GObject*> RadarObject::getSensedObjectsByGtype(GType type)
{
	list<GObject*> result;

	for (GObject* obj : visibleObjects) {
		if (obj->getType() == type) result.push_back(obj);
	}

	return result;
}
