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

class FloorSegment : virtual public GObject, public NoSprite, public RectangleMapBody
{
public:
	MapObjCons(FloorSegment);
	virtual ~FloorSegment();

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::floorSegment; }
	virtual PhysicsLayers getLayers() const { return PhysicsLayers::floor; }
	virtual inline bool getSensor() const { return true; }

	virtual void onContact(GObject* obj) = 0;
	virtual void onEndContact(GObject* obj) = 0;
};

class DirtFloorCave : public FloorSegment
{
public:
	MapObjCons(DirtFloorCave);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

class MineFloor : public FloorSegment
{
public:
	MapObjCons(MineFloor);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

class PressurePlate : public FloorSegment, public RegisterInit<PressurePlate>
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

class Pitfall : public FloorSegment
{
public:
	MapObjCons(Pitfall);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

#endif /* FloorSegment_hpp */
