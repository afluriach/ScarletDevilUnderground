//
//  RoomSensorFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/2/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "RoomSensorFunctions.hpp"
#include "Spawner.hpp"

namespace ai{

SequentialClearSpawn::SequentialClearSpawn(GSpace* space, const ValueMap& args) {
}

void SequentialClearSpawn::update(StateMachine& sm)
{
	RoomSensor* rs = sm.getRoomSensor();

	if (rs->hasPlayer() && !rs->hasEnemies())
	{
		if (rs->activateAllSpawners() == 0)
			sm.pop();
	}
}

}//end NS
