//
//  Spawner.hpp
//  Koumachika
//
//  Created by Toni on 12/29/18.
//
//

#ifndef Spawner_hpp
#define Spawner_hpp

#include "AreaSensor.hpp"

class Agent;

class Spawner :
public AreaSensor,
public ActivateableObject
{
public:
	MapObjCons(Spawner);
    
	void update();

	virtual void activate();
	inline virtual void deactivate() {}

	virtual bool isObstructed() const;
protected:
	ValueMap spawn_args;
	
	unsigned int lastSpawnFrame = 0;
};

#endif /* Spawner_hpp */
