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

class FloorSegmentImpl : public FloorSegment
{
public:
	FloorSegmentImpl(GSpace* space, ObjectIDType id, const ValueMap& args, const string& type);
	virtual ~FloorSegmentImpl();

	virtual inline SpaceFloat getFrictionCoeff() const { return props.traction; }
	virtual string getFootstepSfx() const;
protected:
	floorsegment_properties props;
};

class MovingPlatform:
public FloorSegment,
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

	virtual inline string getSprite() const { return "movingPlatform"; }

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
	virtual inline string getSprite() const { return "icePlatform"; }
	virtual inline string getFootstepSfx() const { return "sfx/footstep_ice.wav"; }
	virtual inline SpaceFloat getFrictionCoeff() const { return 0.2; }
};

class PressurePlate : public FloorSegment, public RegisterInit<PressurePlate>
{
public:
	MapObjCons(PressurePlate);

	void init();

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
protected:
	unordered_set<gobject_ref> crntContacts;
	vector<string> targetNames;
	vector<gobject_ref> target;
};

class Pitfall : public FloorSegment
{
public:
	MapObjCons(Pitfall);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
	virtual void exclusiveFloorEffect(GObject* obj);

	virtual PhysicsLayers getLayers() const { return PhysicsLayers::belowFloor; }
};

class WaterFloor : public FloorSegment
{
public:
	MapObjCons(WaterFloor);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
	virtual void exclusiveFloorEffect(GObject* obj);

	virtual PhysicsLayers getLayers() const { return PhysicsLayers::belowFloor; }
};

#endif /* FloorSegment_hpp */
