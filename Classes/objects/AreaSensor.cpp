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
	RectangleMapBody(args)
{
}

PhysicsLayers AreaSensor::getLayers() const{
    return PhysicsLayers::ground;
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

TrapRoomSensor::TrapRoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space,id,args),
	AreaSensor(space,id,args),
	RegisterInit(this),
	RegisterUpdate(this)
{
	doorNames = splitString(getStringOrDefault(args,"doors",""), " ");
}

void TrapRoomSensor::init()
{
	for (string name : doorNames) {
		Door* d = space->getObjectAs<Door>(name);
		if (d) {
			doors.insert(d);
		}
		else {
			log("TrapRoomSensor: unknown door %s.", name.c_str());
		}
	}
}

void TrapRoomSensor::update()
{
	if (!isLocked && player.isValid() && enemies.size() > 0) {
		for (auto ref : doors) {
			ref.get()->setLocked(true);
		}
		isLocked = true;
	}
	else if(isLocked && enemies.empty())
	{
		for (auto ref : doors) {
			ref.get()->setLocked(false);
		}
		isLocked = false;
	}
}

BossRoomSensor::BossRoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(space, id, args),
	AreaSensor(space, id, args),
	RegisterInit(this),
	RegisterUpdate(this)
{
	bossName = getStringOrDefault(args, "boss", "");
}

void BossRoomSensor::init()
{
	boss = space->getObjectRefAs<Enemy>(bossName);
}

void BossRoomSensor::update()
{
	if (!activated)
	{
		if (boss.isValid() && player.isValid()) {
			space->getSceneAs<PlayScene>()->hud->setEnemyInfo(boss);
			activated = true;
		}
	}
	else
	{
		//If the player ref is not valid, this means the player left the room sensor.

		if (!boss.isValid() ) {
			space->getSceneAs<PlayScene>()->hud->setEnemyInfo(nullptr);
		}
	}
}
