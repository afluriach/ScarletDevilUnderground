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

class PressurePlate : public FloorSegment
{
public:
	PressurePlate(
		GSpace* space,
		ObjectIDType id,
		const ValueMap& args,
		local_shared_ptr<floorsegment_properties> props
	);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);

	void runActivate();
	void runDeactivate();
protected:
	vector<gobject_ref> targets;
	unsigned int contactCount = 0;
};

class Pitfall : public GObject
{
public:
	MapObjCons(Pitfall);

	virtual void exclusiveFloorEffect(GObject* obj);
};

#endif /* FloorSegment_hpp */
