//
//  AreaSensor.hpp
//  Koumachika
//
//  Created by Toni on 1/13/19.
//
//

#ifndef AreaSensor_hpp
#define AreaSensor_hpp

#include "GObject.hpp"

class Door;
class Enemy;
class GhostHeadstone;
class GhostFairyNPC;
class Spawner;

class AreaSensor : public GObject
{
public:
	AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args);
	AreaSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dim);

	inline virtual ~AreaSensor() {}

	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);

	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::areaSensor;}
    virtual PhysicsLayers getLayers() const;

	virtual bool isObstructed() const;

	virtual void onPlayerContact(Player*);
	virtual void onPlayerEndContact(Player*);

	void onEnemyContact(Enemy*);
	void onEnemyEndContact(Enemy*);

	inline virtual void onNPCContact(Agent*) {}
	inline virtual void onNPCEndContact(Agent*) {}

	void onEnvironmentalObjectContact(GObject*);
	void onEnvironmentalObjectEndContact(GObject*);

	inline bool hasEnemies() const { return !enemies.empty(); }
	inline bool hasPlayer() const { return player != nullptr; }
	unsigned int getEnemyCount(type_index t);
protected:
	unordered_set<Enemy*> enemies;
	unordered_map<type_index, unsigned int> enemyCountsByType;
	unordered_set<GObject*> environmentalObjects;
	Player* player = nullptr;
};

class HiddenSubroomSensor :
	public AreaSensor
{
public:
	HiddenSubroomSensor(GSpace* space, ObjectIDType id, const ValueMap& args);

	virtual void onPlayerContact(Player*);
protected:
	int roomID;
	bool activated = false;
};

class RoomSensor : public AreaSensor
{
public:
	RoomSensor(GSpace* space, ObjectIDType id, const ValueMap& obj);
	RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props);

	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);

	virtual void onPlayerContact(Player* p);
	virtual void onPlayerEndContact(Player* p);

	virtual void init();
	virtual void update();

	void updateTrapDoors();
	void updateBoss();
	void activateBossObjects();
	void deactivateBossObjects();
	unsigned int activateAllSpawners();
	unsigned int activateSpawners(type_index t, unsigned int count);
	inline SpaceFloat getTimeInRoom()const { return timeInRoom; }
	inline bool getCleared() const { return isCleared; }
	inline int getID() const { return mapID; }
	
	const int mapID;
protected:
	bool isClearedState();

	vector<string> trapDoorNames;
	vector<string> bossActivationNames;
	string spawnOnClear;

	vector<GObject*> doors;
	vector<GObject*> bossActivations;
	unordered_map<type_index, vector<Spawner*>> spawnersByType;
	SpaceFloat timeInRoom = 0.0;

	string bossName;
	object_ref<Enemy> boss;

	bool isCleared = false;
	bool isBossActive = false;
	bool isTrapActive = false;
};

class GhostHeadstoneSensor : public AreaSensor
{
public:
	MapObjCons(GhostHeadstoneSensor);

	virtual void init();
	void checkActivate();

	virtual void onNPCContact(Agent* agent);
	virtual void onNPCEndContact(Agent* agent);

	virtual void onPlayerContact(Player*);

protected:
	unordered_set<object_ref<GhostFairyNPC>> fairies;
	string targetName;
	gobject_ref target;
	int cost = 0;
};

#endif /* AreaSensor_hpp */
