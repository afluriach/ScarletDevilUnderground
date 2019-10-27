//
//  StateMapping.cpp
//  Koumachika
//
//  Created by Toni on 3/15/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "RoomSensorFunctions.hpp"

using namespace ai;

template <typename T>
static constexpr Function::AdapterType consAdapter()
{
    return [](StateMachine* fsm, const ValueMap& args) -> shared_ptr<Function> {
		return make_shared<T>(fsm, args);
	};
}

#define entry(name,cls) {name, consAdapter<cls>()}
//To make an entry where the name matches the class
#define entry_same(cls) entry(#cls, cls)

const unordered_map<string, Function::AdapterType> Function::adapters = {
	entry_same(Flee),
	entry_same(FollowPath),
	entry_same(IdleWait),
	entry_same(MoveToPoint),
	entry_same(MultiSpawnSequence),
	entry_same(Seek),
	entry_same(SequentialClearSpawn),
	entry_same(TimedSpawnSequence),
	entry_same(Wander)
};

