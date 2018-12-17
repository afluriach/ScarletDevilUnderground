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
#include "Graphics.h"
#include "GSpace.hpp"
#include "value_map.hpp"

FloorSegment::FloorSegment(GSpace* space, ObjectIDType id, const ValueMap& args) :
RectangleMapBody(args)
{

}

FloorSegment::~FloorSegment()
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
	space->removeObjectWithAnimation(obj, pitfallShrinkAction());
}

void Pitfall::onEndContact(GObject* obj)
{
}