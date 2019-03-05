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
#include "GSpace.hpp"
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
	roomID(getIntOrDefault(args, "id", -1)),
	RegisterUpdate<HiddenSubroomSensor>(this)
{

}

void HiddenSubroomSensor::update()
{
	if (player.isValid() && roomID != -1 && !activated)
	{
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
	RegisterUpdate<RoomSensor>(this),
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
			space->addSceneAction(make_hud_action(
				&HUD::setEnemyInfo,
				playScene,
				boss.get()->getProperName(),
				boss.get()->getAttribute(Attribute::hp),
				boss.get()->getAttribute(Attribute::maxHP)
			));

			isBossActive = true;
		}
	}
	else
	{
		//If the player ref is not valid, this means the player left the room sensor.

		if (boss.isValid() && player.isValid()) {
			space->addSceneAction(make_hud_action(
				&HUD::updateEnemyInfo,
				playScene,
				boss.get()->getAttribute(Attribute::hp)
			));
		}

		else {
			space->addSceneAction(make_hud_action(
				&HUD::clearEnemyInfo,
				playScene
			));
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
	space->createObject(Collectible::create(collectible_id::key, point));
	isKeyDrop = false;
}
