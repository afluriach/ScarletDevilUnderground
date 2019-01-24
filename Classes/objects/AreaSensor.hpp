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
	public RectangleMapBody,
	public NoSprite
{
public:
	AreaSensor(GSpace* space, ObjectIDType id, const ValueMap& args);
	inline virtual ~AreaSensor() {}

    virtual inline SpaceFloat getMass() const { return -1.0;}
	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::areaSensor;}
    virtual PhysicsLayers getLayers() const;

	void onPlayerContact(Player*);
	void onPlayerEndContact(Player*);

	void onEnemyContact(Enemy*);
	void onEnemyEndContact(Enemy*);
protected:
	set<object_ref<Enemy>> enemies;
	object_ref<Player> player;
};

class TrapRoomSensor :
	public AreaSensor,
	public RegisterInit<TrapRoomSensor>,
	public RegisterUpdate<TrapRoomSensor>
{
public:
	TrapRoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args);

	void init();
	void update();
protected:
	vector<string> doorNames;
	set<object_ref<Door>> doors;
	bool isLocked = false;
};

class BossRoomSensor :
	public AreaSensor,
	public RegisterInit<BossRoomSensor>,
	public RegisterUpdate<BossRoomSensor>
{
public:
	BossRoomSensor(GSpace* space, ObjectIDType id, const ValueMap& args);

	void init();
	void update();
protected:
	string bossName;
	object_ref<Enemy> boss;
	bool activated = false;
};


#endif /* AreaSensor_hpp */
