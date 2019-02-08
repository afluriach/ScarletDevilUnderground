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

RoomSensor::RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props) :
	GObject(space, id, "", center, 0.0),
	AreaSensor(space,id,center,dimensions),
	RegisterInit<RoomSensor>(this),
	RegisterUpdate<RoomSensor>(this),
	mapID(mapID)
{
	trapDoorNames = splitString(getStringOrDefault(props, "trap_doors", ""), "");
	bossName = getStringOrDefault(props, "boss", "");
}

void RoomSensor::onPlayerContact(Player* p)
{
	AreaSensor::onPlayerContact(p);
	log("Player entered room %d.", mapID);
}

void RoomSensor::onPlayerEndContact(Player* p)
{
	AreaSensor::onPlayerEndContact(p);
	log("Player left room %d.", mapID);
}

void RoomSensor::init()
{
	for (string name : trapDoorNames) {
		Door* d = space->getObjectAs<Door>(name);
		if (d) {
			doors.insert(d);
		}
		else {
			log("TrapRoomSensor: unknown door %s.", name.c_str());
		}
	}
	trapDoorNames.clear();

	boss = space->getObjectRefAs<Enemy>(bossName);
}

void RoomSensor::update()
{
	if(!bossName.empty())
		updateBoss();
	if(doors.size() > 0)
		updateTrapDoors();
}

void RoomSensor::updateTrapDoors()
{
	if (!isTrapActive && player.isValid() && enemies.size() > 0) {
		for (auto ref : doors) {
			ref.get()->setLocked(true);
		}
		isTrapActive = true;
	}
	else if (isTrapActive && enemies.empty())
	{
		for (auto ref : doors) {
			ref.get()->setLocked(false);
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
				boss.get()->getName(),
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
