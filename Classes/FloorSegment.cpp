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