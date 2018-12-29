//
//  TeleportPad.cpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#include "Prefix.h"

#include "TeleportPad.hpp"

TeleportPad::TeleportPad(GSpace* space, ObjectIDType id, const ValueMap& args) :
MapObjForwarding(GObject),
RectangleMapBody(args)
{
}

PhysicsLayers TeleportPad::getLayers() const{
    return PhysicsLayers::ground;
}

void TeleportPad::onContact(GObject* obj)
{
	contacts.insert(obj);
}

void TeleportPad::onEndContact(GObject* obj)
{
	contacts.erase(obj);
}

bool TeleportPad::isObstructed() const {
	return !contacts.empty() || teleportActive;
}

void TeleportPad::setTeleportActive(bool b) {
	teleportActive = b;
}