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

    inline StateMachineObject(const ValueMap& args) : StateMachineObject(nullptr, args) {}
	StateMachineObject(shared_ptr<ai::State> startState, const ValueMap& args);

	inline void update() {
		fsm.update();
	}

	inline void setState(shared_ptr<ai::State> state) {
		fsm.setState(state);
	}

	inline void pushState(shared_ptr<ai::State> state) {
		fsm.push(state);
	}

	inline void clearState() {
		fsm.clearState();
	}


protected:
	ai::StateMachine fsm;
};

class RadarObject : virtual public GObject, RegisterInit<RadarObject>, RegisterUpdate<RadarObject>
{
public:
    inline RadarObject() :
    RegisterInit(this),
    RegisterUpdate(this)
    {}
    
    inline void init(){
        setFovAngle(getDefaultFovAngle());
    }

	virtual float getRadarRadius() const = 0;
	virtual GType getRadarType() const = 0;
    virtual inline float getDefaultFovAngle() const {return 0.0f;}

	virtual inline void onDetect(GObject* other) {
		ctx.callIfExistsNoReturn("onDetect", ctx.makeArgs(other));
	}

	virtual inline void onEndDetect(GObject* other) {
		ctx.callIfExistsNoReturn("onEndDetect", ctx.makeArgs(other));
	}

    void radarCollision(GObject* other);
    void radarEndCollision(GObject* other);
    
    //Find the [visible] object that the agent is most directly facing.
    GObject* getSensedObject();
    
    void setFovAngle(float angle);

	//Create body and add it to space. This assumes BB is rectangle dimensions
	virtual inline void initializeRadar(GSpace& space)
	{
		radar = space.createCircleBody(
			initialCenter,
			getRadarRadius(),
			0.1f,
			getRadarType(),
			PhysicsLayers::all,
			true,
			this
		);
	}
    
    bool isObjectVisible(GObject* otther);

    void update();
    void updateVisibleObjects();
    
	inline void updateRadarPos() {
		radar->setPos(body->getPos());
	}
protected:
    unordered_set<GObject*> objectsInRange;
    unordered_set<GObject*> visibleObjects;
    
    //Field of view angle in radians. This is the maximum angle from the facing
    //direction to any visible target, i.e. half of the actual FOV width.
    //If 0, FOV is not considered and this is a radius sensor.
    float fovAngle = 0.0f;
    float fovScalar = 0.0f;
};

#endif /* AIMixins_hpp */
