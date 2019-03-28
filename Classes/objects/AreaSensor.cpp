//
//  AreaSensor.cpp
//  Koumachika
//
//  Created by Toni on 1/13/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "FairyNPC.hpp"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spawner.hpp"
#include "value_map.hpp"

AreaSensor::AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	RectangleBody(args)
{
}

AreaSensor::AreaSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dim) :
	GObject(space,id,"",center,0.0),
	RectangleBody(dim)
{
}

PhysicsLayers AreaSensor::getLayers() const{
    return PhysicsLayers::all;
}

bool AreaSensor::isObstructed() const
{
	return player.isValid() || !enemies.empty() || !environmentalObjects.empty();
}

void AreaSensor::onPlayerContact(Player* p) {
	player = p;
}

void AreaSensor::onPlayerEndContact(Player* p) {
	player = nullptr;
}

void AreaSensor::onEnemyContact(Enemy*e) {
	enemies.insert(e);

	if (player.isValid()) {
		e->sendAlert(player.get());
	}
}

void AreaSensor::onEnemyEndContact(Enemy* e) {
	enemies.erase(e);
}

void AreaSensor::onEnvironmentalObjectContact(GObject* obj) {
	environmentalObjects.insert(obj);
}

void AreaSensor::onEnvironmentalObjectEndContact(GObject* obj) {
	environmentalObjects.erase(obj);
}

HiddenSubroomSensor::HiddenSubroomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(AreaSensor),
	roomID(getIntOrDefault(args, "id", -1))
{

}

void HiddenSubroomSensor::onPlayerContact(Player* p)
{
	AreaSensor::onPlayerContact(p);

	if (!activated) {
		space->clearSubroomMask(roomID);
		activated = true;
	}
}

RoomSensor::RoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	RoomSensor(
		space,
		id,
		getObjectPos(args),
		RectangleBody::getDimensionsFromMap(args),
		getIntOrDefault(args, "name", -1),
		args
	)
{}

RoomSensor::RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props) :
	GObject(space, id, "", center, 0.0),
	AreaSensor(space,id,center,dimensions),
	RegisterInit<RoomSensor>(this),
	mapID(mapID)
{
	trapDoorNames = splitString(getStringOrDefault(props, "trap_doors", ""), " ");
	spawnerNames = splitString(getStringOrDefault(props, "spawners", ""), " ");
	bossName = getStringOrDefault(props, "boss", "");
	keyWaypointName = getStringOrDefault(props, "key_drop", "");
	isKeyDrop = !keyWaypointName.empty();
}

void RoomSensor::onPlayerContact(Player* p)
{
	AreaSensor::onPlayerContact(p);
	log("Player entered room %d.", mapID);

	for (auto ref : enemies)
	{
		ref.get()->sendAlert(p);
	}
}

void RoomSensor::onPlayerEndContact(Player* p)
{
	AreaSensor::onPlayerEndContact(p);
	log("Player left room %d.", mapID);
}

void RoomSensor::init()
{
	for (string name : trapDoorNames) {
		if (name.empty())
			continue;

		ActivateableObject* d = space->getObjectAs<ActivateableObject>(name);
		if (d) {
			doors.insert(d);
		}
		else {
			log("RoomSensor: unknown trap door %s.", name.c_str());
		}
	}
	trapDoorNames.clear();

	for (string name : spawnerNames) {
		if (name.empty())
			continue;

		Spawner* s = space->getObjectAs<Spawner>(name);
		if (s) {
			spawners.insert(s);
		}
		else {
			log("RoomSensor: unknown spawner %s.", name.c_str());
		}
	}
	spawnerNames.clear();

	boss = space->getObjectRefAs<Enemy>(bossName);
}

void RoomSensor::update()
{
	//shouldn't actually be necessary
	//GObject::update();

	if(!bossName.empty())
		updateBoss();
	if(doors.size() > 0)
		updateTrapDoors();
	if (spawners.size() > 0)
		updateSpawners();

	if (isKeyDrop && !keyWaypointName.empty() && player.isValid() && enemies.empty()) {
		spawnKey();
	}
}

void RoomSensor::updateTrapDoors()
{
	if (!isTrapActive && player.isValid() && enemies.size() > 0) {
		for (auto ref : doors) {
			ref.get()->activate();
		}
		isTrapActive = true;
	}
	else if (isTrapActive && enemies.empty())
	{
		for (auto ref : doors) {
			ref.get()->deactivate();
		}
		isTrapActive = false;
	}
}

void RoomSensor::updateBoss()
{
	PlayScene* playScene = space->getSceneAs<PlayScene>();

	if (!isBossActive)
	{
		if (boss.isValid() && player.isValid()) {
			space->addHudAction(
				&HUD::setEnemyInfo,
				boss.get()->getProperName(),
				boss.get()->getAttribute(Attribute::hp),
				boss.get()->getAttribute(Attribute::maxHP)
			);

			isBossActive = true;
		}
	}
	else
	{
		//If the player ref is not valid, this means the player left the room sensor.

		if (boss.isValid() && player.isValid()) {
			space->addHudAction(
				&HUD::updateEnemyInfo,
				boss.get()->getAttribute(Attribute::hp)
			);
		}

		else {
			space->addHudAction(&HUD::clearEnemyInfo);
			isBossActive = false;
		}
	}
}

void RoomSensor::updateSpawners()
{
	if (enemies.size() == 0)
	{
		for (auto ref : spawners)
		{
			Spawner* s = ref.get();

			if (s->getRemainingSpawns() > 0)
				s->activate();
		}
	}
}

void RoomSensor::spawnKey()
{
	SpaceVect point = space->getWaypoint(keyWaypointName);
	space->createObject(Collectible::create(space, collectible_id::key, point));
	isKeyDrop = false;
}

GhostHeadstoneSensor::GhostHeadstoneSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	AreaSensor(space, id, args),
	RegisterInit<GhostHeadstoneSensor>(this)
{
	targetName = getStringOrDefault(args, "target", "");
	cost = getIntOrDefault(args, "cost", -1);

	if (cost == -1) {
		log("GhostHeadstoneSensor %s: unknown cost.", name.c_str());
	}
}

void GhostHeadstoneSensor::init()
{
	if (targetName.empty()) {
		log("GhostHeadstoneSensor %s, unknown target.", name.c_str());
	}
	else {
		target = space->getObjectRef(targetName);
	}
}

void GhostHeadstoneSensor::checkActivate()
{
	if (target.isValid() && to_int(fairies.size()) >= cost)
	{
		int i = 0;
		auto it = fairies.begin();

		//These objects will actually be removed when removals are processed,
		//but it will not run more than once for this sensor.
		for (; it != fairies.end() && i < cost; ++i, ++it) {
			space->removeObject(*it);
		}

		space->removeObject(target);
		target = nullptr;
	}
}

void GhostHeadstoneSensor::onNPCContact(Agent* agent)
{
	if (auto fairy = dynamic_cast<GhostFairyNPC*>(agent)) {
		fairies.insert(fairy);
	}

	checkActivate();
}

void GhostHeadstoneSensor::onNPCEndContact(Agent* agent)
{
	if (auto fairy = dynamic_cast<GhostFairyNPC*>(agent)) {
		fairies.erase(fairy);
	}
}

void GhostHeadstoneSensor::onPlayerContact(Player* p)
{
	checkActivate();
}
