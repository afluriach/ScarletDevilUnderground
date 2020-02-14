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
			-1.0
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

PressurePlate::PressurePlate(
	GSpace* space,
	ObjectIDType id,
	const ValueMap& args,
	local_shared_ptr<floorsegment_properties> props
) :
	FloorSegment(space, id, args, props)
{
	vector<string> targetNames = splitString(getStringOrDefault(args, "target", ""), " ");
	targets.reserve(targetNames.size());

	for (string _name : targetNames)
	{
		targets.push_back(space->getObjectRef(_name));
	}

}

void PressurePlate::onContact(GObject* obj)
{
	if (contactCount == 0) {
		runActivate();
	}

	++contactCount;
}

void PressurePlate::onEndContact(GObject* obj)
{
	if (contactCount == 1) {
		runDeactivate();
	}

	--contactCount;
}

void PressurePlate::runActivate()
{
	for (gobject_ref _t : targets) {
		if (_t.isValid()) {
			_t.get()->activate();
		}
	}
}

void PressurePlate::runDeactivate()
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
