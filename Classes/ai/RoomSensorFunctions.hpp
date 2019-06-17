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

class Spawner;

namespace ai{

//Try/activate all spawners in room when room is cleared.
class SequentialClearSpawn : public Function {
public:
	SequentialClearSpawn(StateMachine* fsm, const ValueMap& args);

	virtual update_return update();

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
	MultiSpawnSequence(StateMachine* fsm, const ValueMap& args);

	virtual update_return update();

	FuncGetName(MultiSpawnSequence)
protected:
	int entryIdx = 0;
	vector<sequence_entry> spawnEntries;
	unordered_map<type_index, unsigned int> totalSpawns;
};

struct timed_sequence_entry
{
	SpaceFloat startTime;
	vector<Spawner*> spawners;
};

class TimedSpawnSequence : public Function {
public:
	TimedSpawnSequence(StateMachine* fsm, const ValueMap& args);

	virtual update_return update();

	FuncGetName(TimedSpawnSequence)
protected:
	int entryIdx = 0;
	vector<timed_sequence_entry> spawnEntries;
	vector<Spawner*> spawnQueue;
};

} //end NS

#endif /* RoomSensorFunctions_hpp */
