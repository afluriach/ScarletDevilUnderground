//
//  AIMixins.hpp
//  Koumachika
//
//  Created by Toni on 3/14/18.
//
//

#ifndef AIMixins_hpp
#define AIMixins_hpp

#include "AI.hpp"

class StateMachineObject : virtual public GObject, RegisterUpdate<StateMachineObject>
{
public:
	inline StateMachineObject(shared_ptr<ai::State> startState) :
		RegisterUpdate(this),
		fsm(this)
	{
		fsm.push(startState);
	}

	inline void update() {
		fsm.update();
	}

protected:
	ai::StateMachine fsm;
};

class RadarObject : virtual public GObject, RegisterUpdate<RadarObject>
{
public:
	virtual float getRadarRadius() const = 0;
	virtual GType getRadarType() const = 0;

	virtual void onDetect(GObject* other) = 0;
	virtual void onEndDetect(GObject* other) = 0;


	//Create body and add it to space. This assumes BB is rectangle dimensions
	virtual inline void initializeRadar(GSpace& space)
	{
		radar = space.createCircleBody(
			initialCenter,
			getRadarRadius(),
			0.1,
			getRadarType(),
			PhysicsLayers::all,
			true,
			this
		);
	}

	inline RadarObject() : RegisterUpdate(this) {}

	inline void update() {
		updateRadarPos();
	}

	inline void updateRadarPos() {
		radar->setPos(body->getPos());
	}
};

class RadarStateMachineObject : virtual public GObject, StateMachineObject, RadarObject
{
public:
	inline RadarStateMachineObject(shared_ptr<ai::State> startState) : StateMachineObject(startState) {}

	inline virtual void onDetect(GObject* obj) {
		fsm.onDetect(obj);
	}

	inline virtual void onEndDetect(GObject* obj) {
		fsm.onEndDetect(obj);
	}
};

class ScriptedRadar : virtual public RadarObject, virtual public ScriptedObject {
public:
	inline ScriptedRadar() {}
	inline void onDetect(GObject* other) {
		ctx.callIfExistsNoReturn("onDetect", ctx.makeArgs(other));
	}
	inline void onEndDetect(GObject* other) {
		ctx.callIfExistsNoReturn("onEndDetect", ctx.makeArgs(other));
	}
};

//A field of view sensor that tracks the object in front of the sensing object.
class ObjectSensor : virtual public RadarObject
{
public:
	inline ObjectSensor() {}

	//for object sensing
	virtual float getRadarRadius() const { return 2.5; }
	virtual GType getRadarType() const { return GType::objectSensor; }

	static const float coneHalfWidth;

	set<GObject*> inRange;

	inline virtual void onDetect(GObject* other)
	{
		inRange.insert(other);
	}
	inline virtual void onEndDetect(GObject* other)
	{
		inRange.erase(other);
	}

	//Find the object, if any, that is within the cone angle, selecting the one
	//that is closest to the center of the object's direction.
	GObject* getSensedObject();
};


#endif /* AIMixins_hpp */
