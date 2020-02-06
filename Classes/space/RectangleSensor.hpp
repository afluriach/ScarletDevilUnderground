//
//  RectangleSensor.hpp
//  Koumachika
//
//  Created by Toni on 2/6/20.
//
//

#ifndef RectangleSensor_hpp
#define RectangleSesnor_hpp

#include "Sensor.hpp"

class RectangleSensor : public Sensor
{
public:
	friend class GSpace;

	RectangleSensor(
		GObject* agent,
		const SpaceRect& area,
		GType targets,
		unary_gobject_function on_detect,
		unary_gobject_function on_end_detect
	);
	~RectangleSensor();

	virtual void collision(GObject* other);
	virtual void endCollision(GObject* other);

	SpaceVect getPos() const;
	void setPos(SpaceVect p) const;
	void setAngle(SpaceFloat a) const;
	SpaceFloat getAngle() const;

	inline void onDetect(GObject* obj) {
		if (on_detect) on_detect(obj);
	}

	inline void onEndDetect(GObject* obj) {
		if (on_end_detect) on_end_detect(obj);
	}

	inline GType getTargetType() { return targetType; }

protected:
	unordered_set<GObject*> objects;

	unary_gobject_function on_detect;
	unary_gobject_function on_end_detect;

	GType targetType;

	GObject* agent;
	b2Body* body;
	b2Fixture* shape;
};

#endif
