//
//  RoomSensorFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/2/19.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "macros.h"
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

}//end NS
