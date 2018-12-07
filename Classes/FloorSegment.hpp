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
#include "types.h"

class FloorSegment : virtual public GObject, public NoSprite, public RectangleMapBody
{
public:
	MapObjCons(FloorSegment);
	virtual ~FloorSegment();

	virtual inline float getMass() const { return -1.0f; }
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

class Pitfall : public FloorSegment
{
public:
	MapObjCons(Pitfall);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

#endif /* FloorSegment_hpp */
