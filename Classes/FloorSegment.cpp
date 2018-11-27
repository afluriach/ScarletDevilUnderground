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
RegisterUpdate<Pitfall>(this),
FloorSegment(args)
{
}

void Pitfall::update()
{
	SpaceVect pos = getPos();
	SpaceVect dim = getDimensions();

	for (auto it = crntContacts.begin(); it != crntContacts.end(); ++it)
	{
		SpaceVect objPos = (*it)->getPos();

		//Check if center of object is on the pitfall.

		if (abs(objPos.x - pos.x) < dim.x / 2 && abs(objPos.y - pos.y) < dim.y / 2) {
			app->space->removeObjectWithAnimation(*it, pitfallShrinkAction());
		}
	}
}

void Pitfall::onContact(GObject* obj)
{
	crntContacts.insert(obj);
}

void Pitfall::onEndContact(GObject* obj)
{
	crntContacts.erase(obj);
}