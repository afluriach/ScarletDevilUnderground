//
//  FloorSegment.hpp
//  Koumachika
//
//  Created by Toni on 11/26/18.
//
//

#ifndef FloorSegment_hpp
#define FloorSegment_hpp

class FloorSegment : public GObject
{
public:
	FloorSegment(GSpace* space, ObjectIDType id, const ValueMap& args);
	FloorSegment(GSpace* space, ObjectIDType id, const ValueMap& args, SpaceFloat mass);
	virtual ~FloorSegment();

	virtual SpaceFloat getTraction() const;
	virtual string getFootstepSfx() const;

	virtual void onContact(GObject* obj) {};
	virtual void onEndContact(GObject* obj) {};
protected:
	local_shared_ptr<floorsegment_properties> props;
};

class MovingPlatform:
public FloorSegment
{
public:
	static const SpaceFloat defaultSpeed;

	MapObjCons(MovingPlatform);
	virtual ~MovingPlatform();

	virtual void init();
	virtual void update();

	void setWaypoint(size_t idx);
	void setNextWaypoint();

	virtual inline string getSprite() const { return "movingPlatform"; }

	virtual inline SpaceFloat getMaxSpeed() const { return defaultSpeed; }
protected:
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
	virtual inline SpaceFloat getTraction() const { return 0.2; }
};

class PressurePlate : public FloorSegment
{
public:
	MapObjCons(PressurePlate);

	virtual void init();
	virtual inline string getFootstepSfx() const { return "sfx/footstep_ice.wav"; }
	virtual inline SpaceFloat getTraction() const { return 1.0; }

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
protected:
	unordered_set<gobject_ref> crntContacts;
	vector<string> targetNames;
	vector<gobject_ref> target;
};

class Pitfall : public GObject
{
public:
	MapObjCons(Pitfall);

	virtual void exclusiveFloorEffect(GObject* obj);
};

#endif /* FloorSegment_hpp */
