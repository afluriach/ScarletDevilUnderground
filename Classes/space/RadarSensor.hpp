//
//  RadarSensor.hpp
//  Koumachika
//
//  Created by Toni on 6/23/19.
//
//

#ifndef RadarSensor_hpp
#define RadarSesnor_hpp

#include "Sensor.hpp"

struct sensor_attributes
{
	SpaceFloat radius;
	SpaceFloat fovAngle;
	
	bool isEssence;
};

class RadarSensor : public Sensor
{
public:
	friend class GSpace;

	RadarSensor(
		GObject* agent,
		const sensor_attributes& attributes,
		unary_gobject_function on_detect,
		unary_gobject_function on_end_detect
	);
	~RadarSensor();

	virtual void collision(GObject* other);
	virtual void endCollision(GObject* other);

	//Find the [visible] object that the agent is most directly facing.
	GObject* getSensedObject();
	list<GObject*> getSensedObjectsByGtype(GType type);
	SpaceFloat getSensedObjectDistance(GType type);

	SpaceVect getPos() const;
	void setPos(SpaceVect p) const;
	void setAngle(SpaceFloat a) const;
	SpaceFloat getAngle() const;
	void setFovAngle(SpaceFloat angle);
	bool isObjectVisible(GObject* other);

	inline void onDetect(GObject* obj) {
		if (on_detect) on_detect(obj);
	}

	inline void onEndDetect(GObject* obj) {
		if (on_end_detect) on_end_detect(obj);
	}
protected:
	void update();

	list_set<GObject*> objectsInRange;
	list_set<GObject*> visibleObjects;

	unary_gobject_function on_detect;
	unary_gobject_function on_end_detect;

	//Field of view angle in radians. This is the maximum angle from the facing
	//direction to any visible target, i.e. half of the actual FOV width.
	//If 0, FOV is not considered and this is a radius sensor.
	SpaceFloat fovAngle = 0.0;
	SpaceFloat fovScalar = 0.0;

	GObject* agent;
	b2Body* body;
	b2Fixture* shape;

	bool detectEssence = false;
};

#endif
