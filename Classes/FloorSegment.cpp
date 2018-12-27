//
//  FloorSegment.cpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "FloorSegment.hpp"
#include "GSpace.hpp"
#include "value_map.hpp"

FloorSegment::FloorSegment(GSpace* space, ObjectIDType id, const ValueMap& args) :
RectangleMapBody(args)
{

}

FloorSegment::~FloorSegment()
{

}

const SpaceFloat MovingPlatform::defaultSpeed = 1.0;

MovingPlatform::MovingPlatform(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(FloorSegment),
	MaxSpeedImpl(getFloatOrDefault(args, "speed", defaultSpeed)),
	RegisterInit<MovingPlatform>(this),
	RegisterUpdate<MovingPlatform>(this)
{
	pathName = getStringOrDefault(args, "path", "");
}

MovingPlatform::~MovingPlatform()
{
}

void MovingPlatform::init()
{
	if (!pathName.empty()) {
		path = space->getPath(pathName);
		pathName.clear();
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
	distanceToTarget -= App::secondsPerFrame * getMaxSpeed();

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

IcePlatform::IcePlatform(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space, id, args, true),
MovingPlatform(space, id, args)
{
}


DirtFloorCave::DirtFloorCave(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args, true),
FloorSegment(space,id,args)
{
}

void DirtFloorCave::onContact(GObject* obj)
{
}

void DirtFloorCave::onEndContact(GObject* obj)
{
}

MineFloor::MineFloor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args, true),
	FloorSegment(space, id, args)
{
}


void MineFloor::onContact(GObject* obj)
{
}

void MineFloor::onEndContact(GObject* obj)
{
}

IceFloor::IceFloor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args, true),
	FloorSegment(space, id, args)
{
}

void IceFloor::onContact(GObject* obj)
{
}

void IceFloor::onEndContact(GObject* obj)
{
}


PressurePlate::PressurePlate(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args, true),
	FloorSegment(space, id, args),
	RegisterInit<PressurePlate>(this)
{
	targetNames = splitString(getStringOrDefault(args, "target", ""), " ");
}

void PressurePlate::init()
{
	for (string _name : targetNames)
	{
		target.push_back(space->getObjectRefAs<ActivateableObject>(_name));
	}
	
	targetNames.clear();
}

void PressurePlate::onContact(GObject* obj)
{
	bool wasInactive = crntContacts.empty();

	crntContacts.insert(obj);

	if (wasInactive) {
		for (object_ref<ActivateableObject> _t : target) {
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
		for (object_ref<ActivateableObject> _t : target) {
			if (_t.isValid()) {
				_t.get()->deactivate();
			}
		}
	}
}

Pitfall::Pitfall(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(space,id,args, true),
FloorSegment(space,id,args)
{
}

void Pitfall::onContact(GObject* obj)
{
}

void Pitfall::onEndContact(GObject* obj)
{
}

void Pitfall::exclusiveFloorEffect(GObject* obj)
{
	obj->onPitfall();
}
