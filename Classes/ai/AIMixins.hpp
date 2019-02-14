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
	StateMachineObject(const ValueMap& args);
	StateMachineObject(shared_ptr<ai::Function> startState, const ValueMap& arg);

	void update();
	unsigned int addThread(shared_ptr<ai::Function> threadMain);
	void removeThread(unsigned int uuid);
	void removeThread(const string& name);
	void printFSM();
	void setFrozen(bool val);

protected:
	ai::StateMachine fsm;
	bool isFrozen = false;
};

class RadarObject : virtual public GObject, RegisterInit<RadarObject>, RegisterUpdate<RadarObject>
{
public:
	RadarObject();
	
	void init();
	
	virtual SpaceFloat getRadarRadius() const = 0;
	virtual GType getRadarType() const = 0;
    virtual inline SpaceFloat getDefaultFovAngle() const {return 0.0;}
	virtual inline bool hasEssenceRadar() const { return false; }

	virtual void onDetect(GObject* other) = 0;
	virtual void onEndDetect(GObject* other) = 0;

    void radarCollision(GObject* other);
    void radarEndCollision(GObject* other);
    
    //Find the [visible] object that the agent is most directly facing.
    GObject* getSensedObject();
	list<GObject*> getSensedObjects();

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
