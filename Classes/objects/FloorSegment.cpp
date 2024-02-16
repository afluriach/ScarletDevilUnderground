//
//  FloorSegment.cpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#include "Prefix.h"

#include "FloorSegment.hpp"
#include "LuaAPI.hpp"

FloorSegment::FloorSegment(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	local_shared_ptr<floorsegment_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(
			GType::floorSegment,
			PhysicsLayers::floor,
			-1.0,
			true,
			false
		),
		props
	),
	props(props)
{
	sol::init_script_object<FloorSegment>(this, params);
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
	runMethodIfAvailable("onContact", obj);
}

void FloorSegment::onEndContact(GObject* obj)
{
	runMethodIfAvailable("onEndContact", obj);
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
			true,
			false
		),
		nullptr
	)
{
}

void Pitfall::exclusiveFloorEffect(GObject* obj)
{
	obj->onPitfall();
}
