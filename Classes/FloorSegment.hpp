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

class FloorSegment : virtual public GObject, public RectangleMapBody
{
public:
	MapObjCons(FloorSegment);
	virtual ~FloorSegment();

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::floorSegment; }
	virtual PhysicsLayers getLayers() const { return PhysicsLayers::floor; }
	virtual inline bool getSensor() const { return true; }

	virtual inline SpaceFloat getFrictionCoeff() const { return 1.0; }

	virtual void onContact(GObject* obj) = 0;
	virtual void onEndContact(GObject* obj) = 0;
};

class MovingPlatform:
public FloorSegment,
public ImageSprite,
public MaxSpeedImpl,
public RegisterInit<MovingPlatform>, 
public RegisterUpdate<MovingPlatform>
{
public:
	static const SpaceFloat defaultSpeed;

	MapObjCons(MovingPlatform);
	virtual ~MovingPlatform();

	void init();
	void update();

	void setWaypoint(size_t idx);
	void setNextWaypoint();

	virtual inline string imageSpritePath() const { return "sprites/moving_platform.png"; }
	virtual inline float zoom() const { return 3.0f; }

	virtual inline SpaceFloat getMass() const { return 1.0; }

	virtual void onContact(GObject* obj) {};
	virtual void onEndContact(GObject* obj) {};
protected:
	string pathName = "";
	const Path * path = nullptr;
	size_t crntSegment = 0;
	SpaceFloat distanceToTarget = 0.0;
};

class DirtFloorCave : public FloorSegment, public NoSprite
{
public:
	MapObjCons(DirtFloorCave);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

class MineFloor : public FloorSegment, public NoSprite
{
public:
	MapObjCons(MineFloor);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

class IceFloor : public FloorSegment, public NoSprite
{
public:
	static const SpaceFloat frictionCoeff;

	MapObjCons(IceFloor);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);

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
	set<gobject_ref> crntContacts;
	vector<string> targetNames;
	vector<object_ref<ActivateableObject>> target;
};

class Pitfall : public FloorSegment, public NoSprite
{
public:
	MapObjCons(Pitfall);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);

	virtual PhysicsLayers getLayers() const { return PhysicsLayers::belowFloor; }
};

#endif /* FloorSegment_hpp */
