//
//  AreaSensor.cpp
//  Koumachika
//
//  Created by Toni on 1/13/19.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "AreaSensor.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "graphics_context.hpp"
#include "HUD.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "value_map.hpp"

AreaSensor::AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(
		space,
		id,
		MapParams(),
		physics_params(GType::areaSensor, PhysicsLayers::all, -1.0, true),
		nullptr
	)
{
}

AreaSensor::AreaSensor(
	GSpace* space,
	ObjectIDType id,
	const object_params& params, 
	local_shared_ptr<object_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(GType::areaSensor, PhysicsLayers::all, -1.0, true),
		props
	)
{
}

bool AreaSensor::isObstructed() const
{
	return obstacleCount > 0;
}

void AreaSensor::beginContact(GObject* obj)
{
	++obstacleCount;
}

void AreaSensor::endContact(GObject* obj)
{
	--obstacleCount;
}

AreaSensorImpl::AreaSensorImpl(
	GSpace* space,
	ObjectIDType id,
	SpaceRect rect,
	GType targets,
	unary_gobject_function onContact,
	unary_gobject_function onEndContact
) :
	AreaSensor(space,id, object_params(rect), nullptr),
	targets(targets),
	onContact(onContact),
	onEndContact(onEndContact)
{
}

void AreaSensorImpl::beginContact(GObject* obj)
{
	if (bitwise_and_bool(obj->getType(), targets) && onContact) {
		onContact(obj);
	}
}

void AreaSensorImpl::endContact(GObject* obj)
{
	if (bitwise_and_bool(obj->getType(), targets) && onEndContact) {
		onEndContact(obj);
	}
}

HiddenSubroomSensor::HiddenSubroomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(AreaSensor),
	roomID(getIntOrDefault(args, "id", -1))
{

}

void HiddenSubroomSensor::onPlayerContact(Player* p)
{
	AreaSensor::onPlayerContact(p);

	if (!activated) {
		space->addGraphicsAction(&graphics_context::clearSubroomMask, to_uint(roomID));
		activated = true;
	}
}

RoomSensor::RoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	RoomSensor(
		space,
		id,
		SpaceRect(getObjectPos(args),getObjectDimensions(args)),		
		getIntOrDefault(args, "name", -1),
		args
	)
{}

RoomSensor::RoomSensor(
	GSpace* space,
	ObjectIDType id,
	SpaceRect rect,
	int mapID,
	const ValueMap& props
) :
	AreaSensor(space,id, object_params( rect), nullptr),
	mapID(mapID)
{
	trapDoorNames = splitString(getStringOrDefault(props, "trap_doors", ""), " ");
	bossActivationNames = splitString(getStringOrDefault(props, "boss_activation_targets", ""), " ");
	spawnOnClear = getStringOrDefault(props, "spawn_on_clear", "");
	bossName = getStringOrDefault(props, "boss", "");

	space->addRoomSensor(this);
}

RoomSensor::~RoomSensor()
{
	space->removeRoomSensor(this);
}

void RoomSensor::beginContact(GObject* obj)
{
	AreaSensor::beginContact(obj);

	obj->setCrntRoom(this);

	if (obj->getType() == GType::player) {
		onPlayerContact(dynamic_cast<Player*>(obj));
	}
	else if (obj->getType() == GType::enemy) {
		onEnemyContact(dynamic_cast<Enemy*>(obj));
	}
}

void RoomSensor::endContact(GObject* obj)
{
	AreaSensor::endContact(obj);

	RoomSensor* prev = obj->getCrntRoom();

	if (prev == this) {
		obj->setCrntRoom(nullptr);
	}

	if (obj->getType() == GType::player) {
		onPlayerEndContact(dynamic_cast<Player*>(obj));
	}
	else if (obj->getType() == GType::enemy) {
		onEnemyEndContact(dynamic_cast<Enemy*>(obj));
	}
}

void RoomSensor::onPlayerContact(Player* p)
{
	player = p;
	log("Player entered room %d.", mapID);

	for (Enemy* e : enemies){
		e->sendAlert(p);
	}
}

void RoomSensor::onPlayerEndContact(Player* p)
{
	log("Player left room %d.", mapID);
	player = nullptr;
}

void RoomSensor::onEnemyContact(Enemy* e) {
	enemies.insert(e);

	emplaceIfEmpty<string, unsigned int>(enemyCountsByType, e->getClsName(), 0);
	++enemyCountsByType.at(e->getClsName());

	if (player) {
		e->sendAlert(player);
	}
}

void RoomSensor::onEnemyEndContact(Enemy* e) {
	--enemyCountsByType.at(e->getClsName());
	enemies.erase(e);
}

unsigned int RoomSensor::getEnemyCount(string typeName)
{
	return getOrDefault(enemyCountsByType, typeName, to_uint(0));
}

void RoomSensor::init()
{
	GObject::init();

	doors = space->getObjectsAs<GObject>(trapDoorNames);
	trapDoorNames.clear();

	bossActivations = space->getObjectsAs<GObject>(bossActivationNames);
	bossActivationNames.clear();

	boss = space->getObjectRef(bossName);
}

void RoomSensor::update()
{
	GObject::update();

	if (player) timerIncrement(timeInRoom);

	if(!bossName.empty())
		updateBoss();
	if(doors.size() > 0)
		updateTrapDoors();

	if (!isCleared && player && isClearedState()) {
		isCleared = true;

		if (!spawnOnClear.empty() && enemies.empty()) {
			vector<string> spawns = splitString(spawnOnClear, " ");
			for (string s : spawns) {
				space->createDynamicObject(s);
			}
			spawnOnClear.clear();
		}
	}
}

void RoomSensor::updateTrapDoors()
{
	if (!isTrapActive && player && enemies.size() > 0) {
		for (auto* d : doors) {
			d->activate();
		}
		isTrapActive = true;
	}
	else if (isTrapActive && enemies.empty())
	{
		for (auto* d : doors) {
			d->deactivate();
		}
		isTrapActive = false;
	}
}

void RoomSensor::updateBoss()
{
	PlayScene* playScene = space->getSceneAs<PlayScene>();

	if (!isBossActive)
	{
		if (boss.isValid() && player) {
			Enemy* _boss = boss.getAs<Enemy>();
			space->addHudAction(
				&HUD::setEnemyInfo,
				_boss->getProperName(),
				_boss->get(Attribute::hp),
				_boss->get(Attribute::maxHP)
			);

			isBossActive = true;
		}
	}
	else
	{
		//If the player ref is not valid, this means the player left the room sensor.

		if (boss.isValid() && player) {
			space->addHudAction(
				&HUD::updateEnemyInfo,
				boss.getAs<Enemy>()->get(Attribute::hp)
			);
		}

		else {
			space->addHudAction(&HUD::clearEnemyInfo);
			isBossActive = false;
		}
	}
}

void RoomSensor::activateBossObjects()
{
	for (auto obj : bossActivations)
		obj->activate();
}

void RoomSensor::deactivateBossObjects()
{
	for (auto obj : bossActivations)
		obj->deactivate();
}

bool RoomSensor::isClearedState()
{
	return enemies.empty();
}
