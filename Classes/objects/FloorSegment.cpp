//
//  FloorSegment.cpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#include "Prefix.h"

#include "FloorSegment.hpp"
#include "GSpace.hpp"
#include "value_map.hpp"

FloorSegment::FloorSegment(GSpace* space, ObjectIDType id, const ValueMap& args) :
GObject(make_shared<object_params>(space,id,args)),
RectangleBody(args)
{

}

FloorSegment::~FloorSegment()
{

}

FloorSegmentImpl::FloorSegmentImpl(GSpace* space, ObjectIDType id, const ValueMap& args, const string& type) :
	MapObjParams(),
	MapObjForwarding(FloorSegment)
{
	auto it = app::floors.find(type);
	if (it != app::floors.end()) {
		props = it->second;
	}
	else {
		log("Unknown floor type: %s", type);
	}
}

FloorSegmentImpl::~FloorSegmentImpl()
{

}

string FloorSegmentImpl::getFootstepSfx() const {
	return props.sfxRes.empty() ? "" : "sfx/" + props.sfxRes + ".wav";
}

const SpaceFloat MovingPlatform::defaultSpeed = 1.0;

MovingPlatform::MovingPlatform(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(FloorSegment),
	RegisterInit<MovingPlatform>(this)
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

IcePlatform::IcePlatform(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjParams(),
MovingPlatform(space, id, args)
{
}

PressurePlate::PressurePlate(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	FloorSegment(space, id, args),
	RegisterInit<PressurePlate>(this)
{
	targetNames = splitString(getStringOrDefault(args, "target", ""), " ");
}

void PressurePlate::init()
{
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

Pitfall::Pitfall(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjParams(),
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

WaterFloor::WaterFloor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	FloorSegment(space, id, args)
{
}

void WaterFloor::onContact(GObject* obj)
{
}

void WaterFloor::onEndContact(GObject* obj)
{
}

void WaterFloor::exclusiveFloorEffect(GObject* obj)
{
	obj->onPitfall();
}
