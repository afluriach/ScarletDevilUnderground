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

class RoomSensor : public AreaSensor
{
public:
	RoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props);

	virtual void onPlayerContact(Player* p);
	virtual void onPlayerEndContact(Player* p);

	const int mapID;
};

class TrapRoomSensor :
	public RoomSensor,
	public RegisterInit<TrapRoomSensor>,
	public RegisterUpdate<TrapRoomSensor>
{
public:
	TrapRoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props);

	void init();
	void update();
protected:
	vector<string> doorNames;
	set<object_ref<Door>> doors;
	bool isLocked = false;
};

class BossRoomSensor :
	public RoomSensor,
	public RegisterInit<BossRoomSensor>,
	public RegisterUpdate<BossRoomSensor>
{
public:
	BossRoomSensor(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions, int mapID, const ValueMap& props);

	void init();
	void update();
protected:
	string bossName;
	object_ref<Enemy> boss;
	bool activated = false;
};


#endif /* AreaSensor_hpp */
