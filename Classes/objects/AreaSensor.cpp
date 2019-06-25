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
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "physics_context.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spawner.hpp"
#include "value_map.hpp"

AreaSensor::AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParams(), physics_params(args, -1.0))
{
}

AreaSensor::AreaSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dim) :
	GObject(
		make_shared<object_params>(space, id, "", center, 0.0),
		physics_params(dim, -1.0)
	)
{
}

PhysicsLayers AreaSensor::getLayers() const{
    return PhysicsLayers::all;
}

bool AreaSensor::isObstructed() const
{
	return player || !enemies.empty() || !environmentalObjects.empty();
}

void AreaSensor::onPlayerContact(Player* p) {
	player = p;
}

void AreaSensor::onPlayerEndContact(Player* p) {
	player = nullptr;
}

void AreaSensor::onEnemyContact(Enemy*e) {
	enemies.insert(e);

	emplaceIfEmpty<type_index, unsigned int>(enemyCountsByType, typeid(*e), 0);
	++enemyCountsByType.at(typeid(*e));

	if (player) {
		e->sendAlert(player);
	}
}

void AreaSensor::onEnemyEndContact(Enemy* e) {
	--enemyCountsByType.at(typeid(*e));
	enemies.erase(e);
}

void AreaSensor::onEnvironmentalObjectContact(GObject* obj) {
	environmentalObjects.insert(obj);
}

void AreaSensor::onEnvironmentalObjectEndContact(GObject* obj) {
	environmentalObjects.erase(obj);
}

unsigned int AreaSensor::getEnemyCount(type_index t)
{
	return getOrDefault(enemyCountsByType, t, to_uint(0));
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
		getObjectPos(args),
		getObjectDimensions(args),
		getIntOrDefault(args, "name", -1),
		args
	)
{}

RoomSensor::RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props) :
	AreaSensor(space,id,center,dimensions),
	mapID(mapID)
{
	string startState = getStringOrDefault(props, "startState", "");

	trapDoorNames = splitString(getStringOrDefault(props, "trap_doors", ""), " ");
	spawnOnClear = getStringOrDefault(props, "spawn_on_clear", "");
	bossName = getStringOrDefault(props, "boss", "");

	fsm = make_unique<ai::StateMachine>(this);

	if (!startState.empty()) {
		auto f = ai::Function::constructState(startState, fsm.get(), props);
		fsm->addThread(f);
	}
}

void RoomSensor::onPlayerContact(Player* p)
{
	AreaSensor::onPlayerContact(p);
	log("Player entered room %d.", mapID);

	for (Enemy* e : enemies){
		e->sendAlert(p);
	}
}

void RoomSensor::onPlayerEndContact(Player* p)
{
	AreaSensor::onPlayerEndContact(p);
	log("Player left room %d.", mapID);
}

void RoomSensor::init()
{
	GObject::init();

	for (string name : trapDoorNames) {
		if (name.empty())
			continue;

		GObject* d = space->getObject(name);
		if (d) {
			doors.insert(d);
		}
		else {
			log("RoomSensor: unknown trap door %s.", getName());
		}
	}
	trapDoorNames.clear();

	unordered_set<Spawner*> _spawners = space->physicsContext->rectangleQueryByType<Spawner>(
		getPos(),
		getDimensions(),
		GType::areaSensor,
		PhysicsLayers::all
	);

	for (Spawner* s : _spawners) {
		spawners.insert(s);
		type_index _type = s->getSpawnType();
		emplaceIfEmpty<type_index, vector<Spawner*>>(spawnersByType, _type);
		spawnersByType.at(_type).push_back(s);
	}

	boss = space->getObjectRefAs<Enemy>(bossName);
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

		if (boss.isValid() && player) {
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

unsigned int RoomSensor::activateAllSpawners()
{
	unsigned int count = 0;

	for (Spawner* s : spawners){
		count += to_uint(s->spawn().isFuture());
	}

	return count;
}

unsigned int RoomSensor::activateSpawners(type_index t, unsigned int count)
{
	unsigned int result = 0;
	auto it = spawnersByType.find(t);
	if (it == spawnersByType.end()) {
		log("Unknown spawner type %s", t.name());
		return 0;
	}

	vector<int> indicies = space->getRandomShuffle(it->second.size());

	for (int i = 0; i < it->second.size() && result < count; ++i) {
		Spawner* s = it->second.at(indicies.at(i)); 
		if (s->spawn().isFuture()) {
			++result;
		}
	}
	return result;
}

bool RoomSensor::isClearedState()
{
	return enemies.empty() && fsm->getThreadCount() == 0;
}

GhostHeadstoneSensor::GhostHeadstoneSensor(GSpace* space, ObjectIDType id, const ValueMap& args) :
	AreaSensor(space, id, args)
{
	targetName = getStringOrDefault(args, "target", "");
	cost = getIntOrDefault(args, "cost", -1);

	if (cost == -1) {
		log("GhostHeadstoneSensor %s: unknown cost.", getName());
	}
}

void GhostHeadstoneSensor::init()
{
	GObject::init();

	if (targetName.empty()) {
		log("GhostHeadstoneSensor %s, unknown target.", getName());
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
