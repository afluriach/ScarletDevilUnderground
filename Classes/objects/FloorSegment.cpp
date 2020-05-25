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
		physics_params(
			GType::floorSegment,
			PhysicsLayers::floor,
			-1.0,
			true
		),
		props
	),
	props(props)
{	
	vector<string> targetNames = splitString(getStringOrDefault(args, "target", ""), " ");
	targets.reserve(targetNames.size());

	for (string _name : targetNames)
	{
		targets.push_back(space->getObjectRef(_name));
	}
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

void FloorSegment::onContact(GObject* obj)
{
	if (contactCount == 0) {
		runActivate();
	}

	++contactCount;
}

void FloorSegment::onEndContact(GObject* obj)
{
	if (contactCount == 1) {
		runDeactivate();
	}

	--contactCount;
}

void FloorSegment::runActivate()
{
	for (gobject_ref _t : targets) {
		if (_t.isValid()) {
			_t.get()->activate();
		}
	}
}

void FloorSegment::runDeactivate()
{
	for (gobject_ref _t : targets) {
		if (_t.isValid()) {
			_t.get()->deactivate();
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
		physics_params(
			GType::floorSegment,
			PhysicsLayers::belowFloor,
			-1.0,
			true
		),
		nullptr
	)
{
}

void Pitfall::exclusiveFloorEffect(GObject* obj)
{
	obj->onPitfall();
}
