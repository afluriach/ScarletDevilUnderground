//
//  RadarSensor.cpp
//  Koumachika
//
//  Created by Toni on 6/23/19.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "physics_context.hpp"
#include "RadarSensor.hpp"

RadarSensor::RadarSensor(
	GObject* agent,
	const sensor_attributes& attributes,
	unary_gobject_function on_detect,
	unary_gobject_function on_end_detect
) :
	targetType(attributes.targetType),
	agent(agent),
	on_detect(on_detect),
	on_end_detect(on_end_detect)
{
	setFovAngle(attributes.fovAngle);

	tie(shape, body) = agent->space->physicsContext->createCircleBody(
		agent->getPos(),
		attributes.radius,
		0.1,
		attributes.targetType,
		PhysicsLayers::all,
		true,
		this
	);
}

RadarSensor::~RadarSensor()
{
	if (body && shape) {
		agent->space->physicsContext->removeObject(shape, body, false);
	}
}

//Objects that gain/lose visibility because they enter/exit sensor range will be
//processed during the next update.
void RadarSensor::radarCollision(GObject* obj)
{
	if (agent == obj)
		return;

	objectsInRange.insert(obj);

	if (isObjectVisible(obj)) {
		onDetect(obj);
		visibleObjects.insert(obj);
	}

}
void RadarSensor::radarEndCollision(GObject* obj)
{
	if (agent == obj)
		return;

	objectsInRange.erase(obj);

	if (visibleObjects.find(obj) != visibleObjects.end()) {
		onEndDetect(obj);
		visibleObjects.erase(obj);
	}
}

bool RadarSensor::isObjectVisible(GObject* other)
{
	if (other->getInvisible())
		return false;

	bool isFov = fovAngle == 0.0 ? true : ai::isInFieldOfView(agent,  other->getPos(), fovScalar);
	bool isLos = detectEssence ? true : ai::isLineOfSight(agent, other);

	return isFov && isLos;
}

void RadarSensor::setFovAngle(SpaceFloat angle)
{
	if (angle == 0.0) {
		fovAngle = 0.0;
		fovScalar = 0.0;
	}
	else {
		fovScalar = cos(angle);
		fovAngle = angle;
	}
}

void RadarSensor::update()
{
	if (agent) {
		cpBodySetPos(body, agent->getPos());
		cpBodySetAngle(body, agent->getAngle());
	}

	for (GObject* obj : objectsInRange)
	{
		bool currentlyVisible = isObjectVisible(obj);
		bool previouslyVisible = visibleObjects.find(obj) != visibleObjects.end();

		if (currentlyVisible && !previouslyVisible) {
			onDetect(obj);
			visibleObjects.insert(obj);
		}
		else if (!currentlyVisible && previouslyVisible) {
			onEndDetect(obj);
			visibleObjects.erase(obj);
		}
	}
}

GObject* RadarSensor::getSensedObject()
{
	SpaceVect facingUnit = SpaceVect::ray(1, getAngle());

	SpaceFloat bestScalar = -1.0;
	GObject* bestObj = nullptr;

	for (GObject* obj : visibleObjects)
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

list<GObject*> RadarSensor::getSensedObjects()
{
	return list<GObject*>(visibleObjects.begin(), visibleObjects.end());
}

list<GObject*> RadarSensor::getSensedObjectsByGtype(GType type)
{
	list<GObject*> result;

	for (GObject* obj : visibleObjects) {
		if (obj->getType() == type) result.push_back(obj);
	}

	return result;
}

SpaceFloat RadarSensor::getSensedObjectDistance(GType type)
{
	SpaceFloat result = numeric_limits<double>::infinity();

	for (GObject* obj : visibleObjects) {
		if (to_uint(obj->getType()) & to_uint(type)) {
			result = min(result, ai::distanceToTarget(agent, obj));
		}
	}
	return result;
}

SpaceVect RadarSensor::getPos() const {
	return cpBodyGetPos(body);
}

void RadarSensor::setPos(SpaceVect p) const {
	cpBodySetPos(body, p);
}

void RadarSensor::setAngle(SpaceFloat a) const {
	cpBodySetAngle(body, canonicalAngle(a));
}

SpaceFloat RadarSensor::getAngle() const {
	return canonicalAngle(cpBodyGetAngle(body));
}
