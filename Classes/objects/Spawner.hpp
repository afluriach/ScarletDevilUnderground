//
//  Spawner.hpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#ifndef Spawner_hpp
#define Spawner_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "object_ref.hpp"

class Agent;

class Spawner :
virtual public GObject,
public RectangleMapBody,
public NoSprite,
public ActivateableObject
{
public:
	MapObjCons(Spawner);
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
	virtual inline bool getSensor() const { return true; }
    virtual inline GType getType() const {return GType::spawner;}
    virtual PhysicsLayers getLayers() const;

	void onContact(GObject* obj);
	void onEndContact(GObject* obj);
	void update();

	virtual void activate();
	inline virtual void deactivate() {}
protected:
	bool isObstructed() const;

	set<gobject_ref> contacts;
	ValueMap spawn_args;
	
	unsigned int lastSpawnFrame = 0;
};

#endif /* Spawner_hpp */
