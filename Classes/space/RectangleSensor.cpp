//
//  RectangleSensor.cpp
//  Koumachika
//
//  Created by Toni on 2/6/20.
//
//

#include "Prefix.h"

#include "physics_context.hpp"
#include "RectangleSensor.hpp"

RectangleSensor::RectangleSensor(
	GObject* agent,
	const SpaceRect& area,
	GType targets,
	unary_gobject_function on_detect,
	unary_gobject_function on_end_detect
) :
	targetType(targets),
	agent(agent),
	on_detect(on_detect),
	on_end_detect(on_end_detect)
{
	tie(body, shape) = agent->space->physicsContext->createRectangleBody(
		area.center,
		area.dimensions,
		0.0,
		GType::agentSensor,
		PhysicsLayers::all,
		true,
		make_any<Sensor*>(this)
	);
}

RectangleSensor::~RectangleSensor()
{
	if (body && shape) {
		agent->space->physicsContext->removeObject(body);
	}
}

//Objects that gain/lose visibility because they enter/exit sensor range will be
//processed during the next update.
void RectangleSensor::collision(GObject* obj)
{
	if (agent == obj || !bitwise_and_bool(obj->getType(), targetType))
		return;

	if (objects.find(obj) == objects.end()) {
		objects.insert(obj);
		onDetect(obj);
	}
}
void RectangleSensor::endCollision(GObject* obj)
{
	if (agent == obj)
		return;

	if (objects.find(obj) != objects.end()) {
		objects.erase(obj);
		onEndDetect(obj);
	}
}

SpaceVect RectangleSensor::getPos() const {
	return body->GetPosition();
}

void RectangleSensor::setPos(SpaceVect p) const {
	body->SetTransform(toBox2D(p), getAngle());
}

void RectangleSensor::setAngle(SpaceFloat a) const {
	body->SetTransform(toBox2D(getPos()), a);
}

SpaceFloat RectangleSensor::getAngle() const {
	return body->GetAngle();
}
