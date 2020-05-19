//
//  RadarSensor.cpp
//  Koumachika
//
//  Created by Toni on 6/23/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "physics_context.hpp"
#include "RadarSensor.hpp"

RadarSensor::RadarSensor(
	GObject* agent,
	const sensor_attributes& attributes,
	unary_gobject_function on_detect,
	unary_gobject_function on_end_detect
) :
	agent(agent),
	on_detect(on_detect),
	on_end_detect(on_end_detect)
{
	setFovAngle(attributes.fovAngle);

	tie(body, shape) = agent->space->physicsContext->createCircleBody(
		agent->getPos(),
		attributes.radius,
		0.0,
		GType::agentSensor,
		PhysicsLayers::all,
		true,
		make_any<Sensor*>(this)
	);

	emplaceIfEmpty(visibleObjectsByType, GType::bomb);
	emplaceIfEmpty(visibleObjectsByType, GType::player);
	emplaceIfEmpty(visibleObjectsByType, GType::playerBullet);
	emplaceIfEmpty(visibleObjectsByType, GType::enemy);
}

RadarSensor::~RadarSensor()
{
	if (body && shape) {
		agent->space->physicsContext->removeObject(body);
	}
}

//Objects that gain/lose visibility because they enter/exit sensor range will be
//processed during the next update.
void RadarSensor::collision(GObject* obj)
{
	if (agent == obj)
		return;

	if (obj->getType() == GType::playerBullet)
		bulletsInRange.insert(obj);
	else
		objectsInRange.insert(obj);
}
void RadarSensor::endCollision(GObject* obj)
{
	if (agent == obj)
		return;

	if (obj->getType() == GType::playerBullet) {
		//Can be called even if object is not visible. 
		removeVisibleBullet(obj);
	}
	else {
		objectsInRange.erase(obj);

		if (visibleObjects.contains(obj)) {
			removeVisibleObject(obj);
		}
	}
}

bool RadarSensor::isObjectVisible(GObject* other)
{
	if (other->isInvisible())
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
		body->SetTransform(toBox2D(agent->getPos()), agent->getAngle());
	}

	for (GObject* obj : objectsInRange.l)
	{
		bool currentlyVisible = isObjectVisible(obj);
		bool previouslyVisible = visibleObjects.contains(obj);

		if (currentlyVisible && !previouslyVisible) {
			addVisibleObject(obj);
		}
		else if (!currentlyVisible && previouslyVisible) {
			removeVisibleObject(obj);
		}
	}

	for (GObject* obj : bulletsInRange.l)
	{
		if (isObjectVisible(obj)) {
			addVisibleBullet(obj);
		}
	}
	bulletsInRange.clear();
}

void RadarSensor::addVisibleObject(GObject* obj)
{
	auto it = visibleObjectsByType.find(obj->getType());
	if (it == visibleObjectsByType.end())
		return;

	onDetect(obj);
	it->second.insert(obj);
	visibleObjects.insert(obj);
}

void RadarSensor::removeVisibleObject(GObject* obj)
{
	auto it = visibleObjectsByType.find(obj->getType());
	if (it == visibleObjectsByType.end())
		return;

	onEndDetect(obj);
	it->second.erase(obj);
	visibleObjects.erase(obj);
}

void RadarSensor::addVisibleBullet(GObject* obj)
{
	auto it = visibleObjectsByType.find(obj->getType());
	if (it == visibleObjectsByType.end())
		return;

	it->second.insert(obj);
}

void RadarSensor::removeVisibleBullet(GObject* obj)
{
	auto it = visibleObjectsByType.find(obj->getType());
	if (it == visibleObjectsByType.end())
		return;

	it->second.erase(obj);
}

GObject* RadarSensor::getSensedObject()
{
	SpaceVect facingUnit = SpaceVect::ray(1, getAngle());

	SpaceFloat bestScalar = -1.0;
	GObject* bestObj = nullptr;

	for (GObject* obj : visibleObjects.l)
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

const object_list* RadarSensor::getSensedObjectsByGtype(GType type) const
{
	auto it = visibleObjectsByType.find(type);
	return it != visibleObjectsByType.end() ? &it->second.l : nullptr;
}

SpaceFloat RadarSensor::getSensedObjectDistance(GType type)
{
	SpaceFloat result = numeric_limits<double>::infinity();
	const object_list* objs = getSensedObjectsByGtype(type);

	for (GObject* obj : *objs) {
		result = min(result, ai::distanceToTarget(agent, obj));
	}
	return result;
}

SpaceVect RadarSensor::getPos() const {
	return body->GetPosition();
}

void RadarSensor::setPos(SpaceVect p) const {
	body->SetTransform(toBox2D(p), getAngle());
}

void RadarSensor::setAngle(SpaceFloat a) const {
	body->SetTransform(toBox2D(getPos()), a);
}

SpaceFloat RadarSensor::getAngle() const {
	return body->GetAngle();
}
