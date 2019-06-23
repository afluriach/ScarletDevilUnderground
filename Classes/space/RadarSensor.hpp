//
//  RadarSensor.hpp
//  Koumachika
//
//  Created by Toni on 6/23/19.
//
//

#ifndef RadarSensor_hpp
#define RadarSesnor_hpp

struct sensor_attributes
{
	SpaceFloat radius;
	SpaceFloat fovAngle;
	GType targetType;
	
	bool isEssence;
};

class RadarSensor
{
public:
	RadarSensor(
		GObject* agent,
		const sensor_attributes& attributes,
		unary_gobject_function on_detect,
		unary_gobject_function on_end_detect
	);
	~RadarSensor();

	void update();

	void radarCollision(GObject* other);
	void radarEndCollision(GObject* other);

	//Find the [visible] object that the agent is most directly facing.
	GObject* getSensedObject();
	list<GObject*> getSensedObjects();
	list<GObject*> getSensedObjectsByGtype(GType type);
	SpaceFloat getSensedObjectDistance(GType type);

	template<class C>
	inline list<C*> getSensedObjectsByType()
	{
		list<C*> result;

		for (GObject* obj : visibleObjects) {
			C* c = dynamic_cast<C*>(obj);
			if (c) result.push_back(c);
		}

		return result;
	}

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
		if (on_detect) on_detect(obj);
	}

	inline GType getTargetType() { return targetType; }

protected:
	unordered_set<GObject*> objectsInRange;
	unordered_set<GObject*> visibleObjects;

	unary_gobject_function on_detect;
	unary_gobject_function on_end_detect;

	//Field of view angle in radians. This is the maximum angle from the facing
	//direction to any visible target, i.e. half of the actual FOV width.
	//If 0, FOV is not considered and this is a radius sensor.
	SpaceFloat fovAngle = 0.0;
	SpaceFloat fovScalar = 0.0;

	GType targetType;

	GObject* agent;
	cpBody* body;
	cpShape* shape;

	bool detectEssence = false;
};

#endif