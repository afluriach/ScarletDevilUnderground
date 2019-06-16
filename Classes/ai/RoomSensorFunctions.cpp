//
//  RoomSensorFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/2/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "GSpace.hpp"
#include "RoomSensorFunctions.hpp"
#include "Spawner.hpp"
#include "value_map.hpp"

namespace ai{

SequentialClearSpawn::SequentialClearSpawn(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
}

shared_ptr<Function> SequentialClearSpawn::update()
{
	RoomSensor* rs = fsm->getRoomSensor();

	if (rs->hasPlayer() && !rs->hasEnemies())
	{
		if (rs->activateAllSpawners() == 0)
			return nullptr;
	}
	return getThis();
}

MultiSpawnSequence::MultiSpawnSequence(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
	vector<type_index> types = getObjectVector<type_index>(args, &GObject::getTypeIndex, "type");
	vector<int> totalCounts = getObjectVector<int>(args, &boost::lexical_cast<int, string>, "totalCount");

	if (types.size() == totalCounts.size()) {
		vector<int> minCounts = getObjectVector<int>(
			args,
			&boost::lexical_cast<int, string>,
			"minCount",
			1,
			types.size(),
			0
		);
		vector<int> waveCounts = getObjectVector<int>(
			args,
			&boost::lexical_cast<int, string>,
			"waveCount",
			1,
			types.size(),
			1
		);

		for_irange(i, 0, types.size()) {
			spawnEntries.push_back(sequence_entry{
				types.at(i),
				minCounts.at(i),
				waveCounts.at(i),
				totalCounts.at(i)
			});
			fsm->getSpace()->increaseSpawnTotal(types.at(i), totalCounts.at(i));
			totalSpawns.insert_or_assign(types.at(i), 0);
		}
	}
}

shared_ptr<Function> MultiSpawnSequence::update()
{
	RoomSensor* rs = fsm->getRoomSensor();
	bool isCompleted = true;

	if (!rs->hasPlayer())
		return getThis();

	for_irange(i, 0, spawnEntries.size())
	{
		auto entry = spawnEntries.at(i);
		if (totalSpawns.at(entry.type) >= entry.totalCount)
			continue;

		isCompleted = false;

		if (rs->getEnemyCount(entry.type) <= entry.minCount) {
			unsigned int desired = totalSpawns.at(entry.type) + entry.waveCount > entry.totalCount ?
				entry.totalCount - totalSpawns.at(entry.type) :
				entry.waveCount;

			totalSpawns.at(entry.type) += rs->activateSpawners(entry.type, desired);
		}
	}

	return isCompleted ? nullptr : getThis();
}

TimedSpawnSequence::TimedSpawnSequence(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
	vector<SpaceFloat> spawnTimes = getObjectVector<double>(
		args,
		&boost::lexical_cast<SpaceFloat, string>,
		"spawnTime"
	);
	ValueVector spawners = getVector(args, "spawners", 1);

	if (spawnTimes.size() != spawners.size())
		return;

	for_irange(i, 0, spawners.size())
	{
		vector<string> spawnerNames = splitString(spawners.at(i).asString(), " ");
		vector<Spawner*> _spawners = fsm->getSpace()->getObjectsAs<Spawner>(spawnerNames);

		for (string s : spawnerNames) {
		}

		spawnEntries.push_back(timed_sequence_entry{
			spawnTimes.at(i),
			_spawners
		});
	}
}

shared_ptr<Function> TimedSpawnSequence::update()
{
	if (entryIdx >= spawnEntries.size()) {
		return nullptr;
	}

	auto it = spawnQueue.begin();
	while (it != spawnQueue.end())
	{
		if ((*it)->canSpawn()) {
			(*it)->spawn();
			it = spawnQueue.erase(it);
		}
		else {
			++it;
		}
	}

	auto entry = spawnEntries.at(entryIdx);

	if (fsm->getRoomSensor()->getTimeInRoom() >= entry.startTime)
	{
		for (Spawner* s : entry.spawners) {
			if (s->canSpawn()) {
				s->spawn();
			}
			else {
				spawnQueue.push_back(s);
			}
		}

		++entryIdx;
	}
	return getThis();
}

}//end NS
