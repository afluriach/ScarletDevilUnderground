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
#include "physics_context.hpp"

RadarObject::RadarObject()
{}

void RadarObject::_init() {
	setFovAngle(getDefaultFovAngle());
}

void RadarObject::initializeRadar(GSpace& space)
{
    tie(radarShape,radar) = space.physicsContext->createCircleBody(
        prevPos,
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
	updateRadarPos();

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

SpaceFloat RadarObject::getSensedObjectDistance(GType type)
{
	SpaceFloat result = numeric_limits<double>::infinity();

	for (GObject* obj : visibleObjects) {
		if (to_uint(obj->getType()) & to_uint(type)){
			result = min(result, ai::distanceToTarget(this, obj));
		}
	}
	return result;
}
