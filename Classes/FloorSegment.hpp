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
	FloorSegment(const ValueMap& args);
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
	DirtFloorCave(const ValueMap& args);

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
};

class Pitfall : public FloorSegment, RegisterUpdate<Pitfall>
{
public:
	Pitfall(const ValueMap& args);

	void update();

	virtual void onContact(GObject* obj);
	virtual void onEndContact(GObject* obj);
protected:
	unordered_set<GObject*> crntContacts;
};

#endif /* FloorSegment_hpp */
