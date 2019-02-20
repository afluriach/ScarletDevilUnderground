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

void GState::registerUpgrade(PlayerCharacter pc, Attribute at, unsigned int id)
{
	size_t upgradeIndex = AttributeSystem::getUpgradeAttributeIndex(at);

	upgrades.at(to_size_t(pc)).upgrades.at(upgradeIndex) |= make_bitfield<AttributeSystem::upgradeCount>(id);
}

bool GState::isUpgradeAcquired(PlayerCharacter pc, Attribute at, unsigned int id)
{
	size_t upgradeIndex = AttributeSystem::getUpgradeAttributeIndex(at);

	return upgrades.at(to_size_t(pc)).upgrades.at(upgradeIndex)[id];
}

float GState::getUpgradeLevel(PlayerCharacter pc, Attribute at)
{
	size_t upgradeIndex = AttributeSystem::getUpgradeAttributeIndex(at);

	bitset<4>& b = upgrades.at(to_size_t(pc)).upgrades.at(upgradeIndex);

	return b.count();
}
