//
//  FloorSegment.cpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#include "Prefix.h"

#include "FloorSegment.hpp"
#include "value_map.hpp"

FloorSegment::FloorSegment(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<floorsegment_properties> props
) :
	GObject(
		space,
		id,
		MapParams(),
		MapRectPhysSensor(
			GType::floorSegment,
			PhysicsLayers::floor,
			props->platform ? 0.0 : -1.0
		)
	),
	props(props)
{	
}

FloorSegment::~FloorSegment()
{

}

string FloorSegment::getFootstepSfx() const {
	return props->sfxRes.empty() ? "" : "sfx/" + props->sfxRes + ".wav";
}

SpaceFloat FloorSegment::getTraction() const {
	return props->traction;
}

const SpaceFloat MovingPlatform::defaultSpeed = 1.0;

MovingPlatform::MovingPlatform(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<floorsegment_properties> props
) :
	FloorSegment(space,id,args,props)
{
}

MovingPlatform::~MovingPlatform()
{
}

//string MovingPlatform::getSprite() const {
//	return props->sprite;
//}

void MovingPlatform::init()
{
	GObject::init();

	string name = getName();
	if (!name.empty()) {
		path = space->getPath(name);
	}
	else {
		log("MovingPlatform::init: path name not provided!");
	}

	if (path && path->size() > 1)
	{
		setWaypoint(0);
	}
	else {
		log("MovingPlatform::init: empty path!");
	}

}

void MovingPlatform::update()
{
	GObject::update();

	distanceToTarget -= app::params.secondsPerFrame * getMaxSpeed();

	if (path && distanceToTarget <= 0) {
		setNextWaypoint();
	}
}

void MovingPlatform::setNextWaypoint()
{
	size_t next = crntSegment + 1;
	if (next >= path->size())
		next = 0;

	setWaypoint(next);
}

void MovingPlatform::setWaypoint(size_t idx)
{
	crntSegment = idx;

	size_t next = crntSegment + 1;
	if (next >= path->size())
		next = 0;

	distanceToTarget = (path->at(next) - path->at(crntSegment)).length();

	SpaceVect dir = (path->at(next) - path->at(crntSegment)).normalizeSafe();
	setPos(path->at(idx));
	setVel(dir*getMaxSpeed());
}

PressurePlate::PressurePlate(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<floorsegment_properties> props
) :
	FloorSegment(space, id, args, props)
{
	targetNames = splitString(getStringOrDefault(args, "target", ""), " ");
}

void PressurePlate::init()
{
	GObject::init();

	for (string _name : targetNames)
	{
		target.push_back(space->getObjectRef(_name));
	}
	
	targetNames.clear();
}

void PressurePlate::onContact(GObject* obj)
{
	bool wasInactive = crntContacts.empty();

	crntContacts.insert(obj);

	if (wasInactive) {
		for (gobject_ref _t : target) {
			if (_t.isValid()) {
				_t.get()->activate();
			}
		}
	}
}

void PressurePlate::onEndContact(GObject* obj)
{
	bool wasActive = !crntContacts.empty();

	crntContacts.erase(obj);

	if (wasActive && crntContacts.empty()) {
		for (gobject_ref _t : target) {
			if (_t.isValid()) {
				_t.get()->deactivate();
			}
		}
	}
}

Pitfall::Pitfall(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args
) :
	GObject(
		space,
		id,
		MapParams(),
		MapRectPhysSensor(
			GType::floorSegment,
			PhysicsLayers::belowFloor,
			-1.0
		)
	)
{
}

void Pitfall::exclusiveFloorEffect(GObject* obj)
{
	obj->onPitfall();
}
