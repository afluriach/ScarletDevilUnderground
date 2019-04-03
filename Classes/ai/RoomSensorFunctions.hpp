//
//  RoomSensorFunctions.hpp
//  Koumachika
//
//  Created by Toni on 4/2/19.
//
//

#ifndef RoomSensorFunctions_hpp
#define RoomSensorFunctions_hpp

#include "AI.hpp"

namespace ai{

//Try/activate all spawners in room when room is cleared.
class SequentialClearSpawn : public Function {
public:
	SequentialClearSpawn(GSpace* space, const ValueMap& args);

	virtual void update(StateMachine& sm);

	FuncGetName(SequentialClearSpawn)
};

struct sequence_entry
{
	type_index type;
	//the threshold before spawning more. Default 0, meaning there must be zero
	//remaining of the given type before spawning more.
	int minCount;
	//The quantity of enemies to try to spawn at once.
	int waveCount;
	//The total number of this kind that can be spawned.
	int totalCount;
};

class MultiSpawnSequence : public Function {
public:
	MultiSpawnSequence(GSpace* space, const ValueMap& args);

	virtual void update(StateMachine& sm);

	FuncGetName(MultiSpawnSequence)
protected:
	int entryIdx = 0;
	vector<sequence_entry> spawnEntries;
	unordered_map<type_index, unsigned int> totalSpawns;
};

} //end NS

#endif /* RoomSensorFunctions_hpp */
