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
#include "GObjectMixins.hpp"
#include "object_ref.hpp"

class Door;
class Enemy;
class Spawner;

class AreaSensor :
	virtual public GObject,
	public RectangleBody,
	public NoSprite
{
public:
	AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args);
	AreaSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dim);

	inline virtual ~AreaSensor() {}

    virtual inline SpaceFloat getMass() const { return -1.0;}
	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::areaSensor;}
    virtual PhysicsLayers getLayers() const;

	virtual bool isObstructed() const;

	virtual void onPlayerContact(Player*);
	virtual void onPlayerEndContact(Player*);

	void onEnemyContact(Enemy*);
	void onEnemyEndContact(Enemy*);

	void onEnvironmentalObjectContact(GObject*);
	void onEnvironmentalObjectEndContact(GObject*);
protected:
	set<object_ref<Enemy>> enemies;
	set<gobject_ref> environmentalObjects;
	object_ref<Player> player;
};

class HiddenSubroomSensor :
	public AreaSensor,
	public RegisterUpdate<HiddenSubroomSensor>
{
public:
	HiddenSubroomSensor(GSpace* space, ObjectIDType id, const ValueMap& args);

	void update();
protected:
	int roomID;
	bool activated = false;
};

class RoomSensor :
	public AreaSensor,
	public RegisterInit<RoomSensor>,
	public RegisterUpdate<RoomSensor>
{
public:
	RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props);

	virtual void onPlayerContact(Player* p);
	virtual void onPlayerEndContact(Player* p);

	void init();
	void update();

	void updateTrapDoors();
	void updateBoss();
	void updateSpawners();

	void spawnKey();
	
	const int mapID;
protected:
	vector<string> trapDoorNames;
	vector<string> spawnerNames;
	set<object_ref<ActivateableObject>> doors;
	set<object_ref<Spawner>> spawners;
	bool isTrapActive = false;

	string bossName;
	object_ref<Enemy> boss;
	bool isBossActive = false;

	string keyWaypointName;
	bool isKeyDrop = false;
};

#endif /* AreaSensor_hpp */
