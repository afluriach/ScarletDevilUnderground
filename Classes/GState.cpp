//
//  GState.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
#include "GState.hpp"
#include "util.h"

vector<bool> GState::profileSlotsInUse;

void GState::initProfiles()
{
	set<string> profiles = io::getProfiles();
	profileSlotsInUse.clear();

	for (int i = 1; i <= maxProfiles; ++i)
	{
		profileSlotsInUse.push_back(profiles.find(string(boost::str(boost::format("profile%d") % i))) != profiles.end());
	}
}

void GState::addItem(string name)
{
	itemRegistry.insert(name);
}

bool GState::hasItem(string name)
{
	return itemRegistry.find(name) != itemRegistry.end();
}

void GState::registerMushroomAcquired(int id)
{
	mushroomFlags |= (1 << id);
}

bool GState::isMushroomAcquired(int id)
{
	return (mushroomFlags >> id) & 1;
}

void GState::setBlueFairyLevel(int id)
{
	blueFairies = id;
}

int GState::getBlueFairyLevel()
{
	return blueFairies;
}

void GState::registerChamberAvailable(ChamberID id)
{
	if (id > ChamberID::invalid_id && id < ChamberID::end) {
		chambersAvailable.at(to_size_t(id)) = true;
	}
	else {
		log("GState::registerChamberAvailable: invalid ID %d", id);
	}
}

bool GState::isChamberAvailable(ChamberID id)
{
	if (id > ChamberID::invalid_id && id < ChamberID::end) {
		return chambersAvailable.at(to_size_t(id));
	}
	else {
		log("GState::isChamberAvailable: invalid ID %d", id);
		return false;
	}
}

bool GState::isChamberCompleted(ChamberID id)
{
	if (id > ChamberID::invalid_id && id < ChamberID::end) {
		return chamberStats.at(to_size_t(id)).timesCompleted > 0;
	}
	else {
		log("GState::isChamberCompleted: invalid ID %d", id);
		return false;
	}
}

void GState::registerUpgrade(Attribute at, unsigned int id)
{
	upgrades.upgrades.at(to_size_t(at)) |= make_bitfield<AttributeSystem::upgradeCount>(id);
}

bool GState::isUpgradeAcquired(Attribute at, unsigned int id)
{
	return upgrades.upgrades.at(to_size_t(at))[id];
}

float GState::getUpgradeLevel(Attribute at)
{
	return upgrades.upgrades.at(to_size_t(at)).count();
}
