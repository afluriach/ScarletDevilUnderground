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
MapObjParams(),
MapObjForwarding(AreaSensor)
{
}

bool TeleportPad::isObstructed() const {
	return AreaSensor::isObstructed() || teleportActive;
}

void TeleportPad::setTeleportActive(bool b) {
	teleportActive = b;
}