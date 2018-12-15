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
#include "GObject.hpp"
#include "GObjectMixins.hpp"

class StateMachineObject : virtual public GObject, RegisterUpdate<StateMachineObject>
{
public:
    inline StateMachineObject(const ValueMap& args) : StateMachineObject(nullptr, args) {}
	StateMachineObject(shared_ptr<ai::Function> startState, const ValueMap& arg);

	inline void update() {
		if(!isFrozen)
			fsm.update();
	}

	inline unsigned int addThread(shared_ptr<ai::Function> threadMain) {
		return fsm.addThread(threadMain);
	}
    
    inline void removeThread(unsigned int uuid){
        fsm.removeThread(uuid);
    }

    inline void removeThread(const string& name){
        fsm.removeThread(name);
    }
    
    inline void printFSM(){
        log("%s", fsm.toString().c_str());
    }

	inline void setFrozen(bool val) {
		isFrozen = val;
	}

protected:
	ai::StateMachine fsm;
	bool isFrozen = false;
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

	virtual SpaceFloat getRadarRadius() const = 0;
	virtual GType getRadarType() const = 0;
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}

	virtual void onDetect(GObject* other);
	virtual void onEndDetect(GObject* other);

    void radarCollision(GObject* other);
    void radarEndCollision(GObject* other);
    
    //Find the [visible] object that the agent is most directly facing.
    GObject* getSensedObject();
	list<GObject*> getSensedObjects() {
		return list<GObject*>(visibleObjects.begin(), visibleObjects.end());
	}
    
    void setFovAngle(SpaceFloat angle);

	//Create body and add it to space. This assumes BB is rectangle dimensions
	virtual void initializeRadar(GSpace& space);
    
    bool isObjectVisible(GObject* otther);

    void update();    
protected:
    unordered_set<GObject*> objectsInRange;
    unordered_set<GObject*> visibleObjects;
    
    //Field of view angle in radians. This is the maximum angle from the facing
    //direction to any visible target, i.e. half of the actual FOV width.
    //If 0, FOV is not considered and this is a radius sensor.
	SpaceFloat fovAngle = 0.0;
	SpaceFloat fovScalar = 0.0;
};

#endif /* AIMixins_hpp */
