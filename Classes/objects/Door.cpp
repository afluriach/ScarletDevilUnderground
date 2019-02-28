//
//  Door.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "Door.hpp"
#include "enum.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "macros.h"
#include "Player.hpp"
#include "value_map.hpp"

Door::Door(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RectangleBody(args),
	RegisterInit<Door>(this)
{
	locked = getBoolOrDefault(args, "locked", false);
	stairs = getBoolOrDefault(args, "stairs", false);
	path = getBoolOrDefault(args, "path", false);
	entryDirection = stringToDirection(getStringOrDefault(args, "dir", "none"));
	destination = getStringOrDefault(args, "dest", "");
	destinationMap = getStringOrDefault(args, "dest_map", "");
	
	string _type = getStringOrDefault(args, "door_type", "pair");
	if (_type == "one_way_source")
		doorType = door_type::one_way_source;
	else if (_type == "one_way_destination")
		doorType = door_type::one_way_destination;
	else
		doorType = door_type::pair;

	int sealed_until = getIntOrDefault(args, "sealed_until_completed", 0);
	if (sealed_until != 0) {
		sealed = !App::crntState->isChamberCompleted(static_cast<ChamberID>(sealed_until));
	}

	setInitialAngle(float_pi / 2.0);
}

void Door::init()
{
	if (!destination.empty() && destinationMap.empty()) {
		adjacent = space->getObjectAs<Door>(destination);

		if (!adjacent.isValid()) {
			log("Door %s, unknown destination %s.", getName().c_str(), destination.c_str());
		}
	}

	if (!adjacent.isValid() && destinationMap.empty()) {
		adjacent = dynamic_cast<Door*>(space->queryAdjacentTiles(
			getPos(),
			GType::environment,
			PhysicsLayers::all,
			type_index(typeid(Door))
		));
	}

	if (locked) {
		space->setSpriteColor(spriteID, Color3B::RED);
	}

	if ((doorType == door_type::one_way_destination || doorType == door_type::one_way_source) && adjacent.isValid()) {
		space->setSpriteTexture(spriteID, "sprites/door_oneway_" + getDoorDirectionString() + ".png");
	}
	else if (stairs) {
		space->setSpriteTexture(spriteID, "sprites/stairs.png" );
	}
	else if (path) {
		space->setSpriteVisible(spriteID, false);
	}
}

PhysicsLayers Door::getLayers() const{
	return PhysicsLayers::all;
}

Direction Door::getDoorDirection() const
{
	if (doorType == door_type::one_way_source) {
		return toDirection(ai::directionToTarget(this, adjacent.get()->getPos()));
	}
	else if (doorType == door_type::one_way_destination) {
		return toDirection(ai::directionToTarget(this, adjacent.get()->getPos()).rotate(float_pi));
	}
	else {
		return Direction::none;
	}
}

string Door::getDoorDirectionString() const
{
	return directionToString(getDoorDirection());
}

string Door::imageSpritePath() const {
	return "sprites/door.png";
}

bool Door::canInteract()
{
	Player* p = space->getObjectAs<Player>("player");

	return doorType != door_type::one_way_destination &&
		(adjacent.isValid() || !destinationMap.empty()) &&
		!sealed &&
		(!locked || p->getKeyCount() > 0)
	;
}

void Door::interact()
{
	Player* p = space->getObjectAs<Player>("player");

	if (!locked && !sealed) {
		p->useDoor(this);
	}
	else if(locked){
		p->useKey();
		locked = false;
		space->setSpriteColor(spriteID, Color3B::WHITE);
	}
}

string Door::interactionIcon()
{
	if (stairs)
		return "sprites/stairs.png";
	else if (path)
		return "sprites/dirt_path.png";
	else if (locked)
		return "sprites/key.png";
	else
		return "sprites/door.png";
}

void Door::activate()
{
	setSealed(true);
}

void Door::deactivate()
{
	setSealed(false);
}

void Door::setSealed(bool b)
{
	sealed = b;

	if (spriteID != 0) {
		space->setSpriteTexture(spriteID, b ? "sprites/door_locked.png" : "sprites/door.png");
	}
}

Door* Door::getAdjacent()
{
	return adjacent.get();
}

SpaceVect Door::getEntryPosition()
{
	return getPos() + SpaceVect::ray(1.0, dirToPhysicsAngle(entryDirection));
}

Direction Door::getEntryDirection()
{
	return entryDirection;
}

Barrier::Barrier(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	RegisterInit<Barrier>(this)
{
	sealed = getBoolOrDefault(args, "sealed", false);
	setInitialAngle(float_pi / 2.0);
}

void Barrier::init()
{
	setSealed(sealed);
}

PhysicsLayers Barrier::getLayers() const
{
	return PhysicsLayers::all;
}

void Barrier::setSealed(bool b)
{
	cpShapeSetSensor(bodyShape, !b);
	space->setSpriteVisible(spriteID, b);
}

void Barrier::activate()
{
	setSealed(true);
}

void Barrier::deactivate()
{
	setSealed(false);
}
