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
	static const unsigned int defaultSpawnLimit;

	MapObjCons(Spawner);
    
	virtual void activate();
	inline virtual void deactivate() {}

	virtual bool isObstructed() const;

	type_index getSpawnType() const;
	int getRemainingSpawns() const;
	int getSpawnLimit() const;
	int getSpawnCount() const;
protected:
	ValueMap spawn_args;
	
	unsigned int lastSpawnFrame = 0;
	unsigned int spawnCount = 0;
	unsigned int spawnLimit;
};

#endif /* Spawner_hpp */
