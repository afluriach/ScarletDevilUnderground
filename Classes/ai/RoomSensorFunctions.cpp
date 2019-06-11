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

MultiSpawnSequence::MultiSpawnSequence(GSpace* space, const ValueMap& args)
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
			space->increaseSpawnTotal(types.at(i), totalCounts.at(i));
			totalSpawns.insert_or_assign(types.at(i), 0);
		}
	}
}

void MultiSpawnSequence::update(StateMachine& sm)
{
	RoomSensor* rs = sm.getRoomSensor();
	bool isCompleted = true;

	if (!rs->hasPlayer()) return;

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

	if (isCompleted)
		sm.pop();
}

TimedSpawnSequence::TimedSpawnSequence(GSpace* space, const ValueMap& args)
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
		vector<Spawner*> _spawners = space->getObjectsAs<Spawner>(spawnerNames);

		for (string s : spawnerNames) {
		}

		spawnEntries.push_back(timed_sequence_entry{
			spawnTimes.at(i),
			_spawners
		});
	}
}

void TimedSpawnSequence::update(StateMachine& sm)
{
	if (entryIdx >= spawnEntries.size()) {
		sm.pop();
		return;
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

	if (sm.getRoomSensor()->getTimeInRoom() >= entry.startTime)
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
}

}//end NS
