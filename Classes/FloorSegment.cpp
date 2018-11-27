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

FloorSegment::FloorSegment(const ValueMap& args) :
RectangleMapBody(args)
{

}

FloorSegment::~FloorSegment()
{

}


DirtFloorCave::DirtFloorCave(const ValueMap& args) :
GObject(args, true),
FloorSegment(args)
{
}

void DirtFloorCave::onContact(GObject* obj)
{
}

void DirtFloorCave::onEndContact(GObject* obj)
{
}

Pitfall::Pitfall(const ValueMap& args) : 
GObject(args, true),
FloorSegment(args)
{
}


void Pitfall::onContact(GObject* obj)
{
	app->space->removeObjectWithAnimation(obj, pitfallShrinkAction());
}

void Pitfall::onEndContact(GObject* obj)
{
}