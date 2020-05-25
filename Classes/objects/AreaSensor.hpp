//
//  AreaSensor.hpp
//  Koumachika
//
//  Created by Toni on 1/13/19.
//
//

#ifndef AreaSensor_hpp
#define AreaSensor_hpp

class AreaSensor : public GObject
{
public:
	AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args);
	AreaSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dim);

	inline virtual ~AreaSensor() {}

	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);

	virtual bool isObstructed() const;
protected:
	unsigned int obstacleCount = 0;
};

class AreaSensorImpl : public AreaSensor
{
public:
	AreaSensorImpl(
		GSpace* space,
		ObjectIDType id,
		SpaceRect rect,
		GType targets,
		unary_gobject_function onContact,
		unary_gobject_function onEndContact
	);

	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);
protected:
	GType targets;
	unary_gobject_function onContact;
	unary_gobject_function onEndContact;
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

	virtual ~RoomSensor();

	virtual void beginContact(GObject* obj);
	virtual void endContact(GObject* obj);

	void onEnemyContact(Enemy*);
	void onEnemyEndContact(Enemy*);

	virtual void onPlayerContact(Player* p);
	virtual void onPlayerEndContact(Player* p);

	virtual void init();
	virtual void update();

	void updateTrapDoors();
	void updateBoss();
	void activateBossObjects();
	void deactivateBossObjects();
	inline SpaceFloat getTimeInRoom()const { return timeInRoom; }
	inline bool getCleared() const { return isCleared; }
	inline int getID() const { return mapID; }

	inline bool hasEnemies() const { return !enemies.empty(); }
	inline bool hasPlayer() const { return player != nullptr; }
	unsigned int getEnemyCount(string typeName);
	
	const int mapID;
protected:
	bool isClearedState();

	unordered_set<Enemy*> enemies;
	unordered_map<string, unsigned int> enemyCountsByType;
	Player* player = nullptr;

	vector<string> trapDoorNames;
	vector<string> bossActivationNames;
	string spawnOnClear;

	vector<GObject*> doors;
	vector<GObject*> bossActivations;
	SpaceFloat timeInRoom = 0.0;

	string bossName;
	gobject_ref boss;

	bool isCleared = false;
	bool isBossActive = false;
	bool isTrapActive = false;
};

#endif /* AreaSensor_hpp */
