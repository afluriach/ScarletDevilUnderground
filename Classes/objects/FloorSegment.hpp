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
	FloorSegment(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		local_shared_ptr<floorsegment_properties> props
	);
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

	MovingPlatform(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		local_shared_ptr<floorsegment_properties> props
	);
	virtual ~MovingPlatform();

//	virtual string getSprite() const;

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

class PressurePlate : public FloorSegment
{
public:
	PressurePlate(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		local_shared_ptr<floorsegment_properties> props
	);

	virtual void init();

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
