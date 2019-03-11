//
//  FloorSegment.hpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#ifndef FloorSegment_hpp
#define FloorSegment_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "object_ref.hpp"
#include "types.h"

class FloorSegment : virtual public GObject, public RectangleBody
{
public:
	MapObjCons(FloorSegment);
	virtual ~FloorSegment();

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::floorSegment; }
	virtual PhysicsLayers getLayers() const { return PhysicsLayers::floor; }
	virtual inline bool getSensor() const { return true; }

	virtual inline SpaceFloat getFrictionCoeff() const { return 1.0; }
	
	virtual inline string getFootstepSfx() const { return ""; }

	virtual void onContact(GObject* obj) {};
	virtual void onEndContact(GObject* obj) {};
	virtual inline void exclusiveFloorEffect(GObject* obj) {}
};

class MovingPlatform:
public FloorSegment,
public ImageSprite,
public MaxSpeedImpl,
public RegisterInit<MovingPlatform>
{
public:
	static const SpaceFloat defaultSpeed;

	MapObjCons(MovingPlatform);
	virtual ~MovingPlatform();

	void init();
	virtual void update();

	void setWaypoint(size_t idx);
	void setNextWaypoint();

	virtual inline string imageSpritePath() const { return "sprites/moving_platform.png"; }
	virtual float zoom() const;

	virtual inline SpaceFloat getMass() const { return 1.0; }
protected:
	string pathName = "";
	const Path * path = nullptr;
	size_t crntSegment = 0;
	SpaceFloat distanceToTarget = 0.0;
};

class IcePlatform : public MovingPlatform
{
public:
	MapObjCons(IcePlatform);
	virtual inline string imageSpritePath() const { return "sprites/ice_platform.png"; }
	virtual inline float zoom() const { return 2.0f; }

	virtual inline string getFootstepSfx() const { return "sfx/footstep_ice.wav"; }

	virtual inline SpaceFloat getFrictionCoeff() const { return 0.2; }
};

class DirtFloorCave : public FloorSegment, public NoSprite
{
public:
	MapObjCons(DirtFloorCave);
};

class BridgeFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(BridgeFloor);
};

class MineFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(MineFloor);

	virtual inline string getFootstepSfx() const { return "sfx/footstep_cave.wav"; }
};

class IceFloor : public FloorSegment, public NoSprite
{
public:
	static const SpaceFloat frictionCoeff;

	MapObjCons(IceFloor);

	virtual inline string getFootstepSfx() const { return "sfx/footstep_ice.wav"; }

	virtual inline SpaceFloat getFrictionCoeff() const { return 0.25; }
};

class PressurePlate : public FloorSegment, public RegisterInit<PressurePlate>, public NoSprite
{
public:
	MapObjCons(PressurePlate);

	void init();

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
protected:
	unordered_set<gobject_ref> crntContacts;
	vector<string> targetNames;
	vector<object_ref<ActivateableObject>> target;
};

class Pitfall : public FloorSegment, public NoSprite
{
public:
	MapObjCons(Pitfall);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
	virtual void exclusiveFloorEffect(GObject* obj);

	virtual PhysicsLayers getLayers() const { return PhysicsLayers::belowFloor; }
};

class WaterFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(WaterFloor);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
	virtual void exclusiveFloorEffect(GObject* obj);

	virtual PhysicsLayers getLayers() const { return PhysicsLayers::belowFloor; }
};


class GrassFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(GrassFloor);

	virtual inline string getFootstepSfx() const { return "sfx/footstep_grass.wav"; }
};

class StoneFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(StoneFloor);

	virtual inline string getFootstepSfx() const { return "sfx/footstep_stone.wav"; }
};

class SandFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(SandFloor);

	virtual inline string getFootstepSfx() const { return "sfx/footstep_sand.wav"; }
};


#endif /* FloorSegment_hpp */
