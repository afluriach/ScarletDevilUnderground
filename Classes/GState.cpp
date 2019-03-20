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
#include "Player.hpp"
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
	if (isValidChamber(id)) {
		chambersAvailable.at(to_size_t(id)) = true;
	}
	else {
		log("GState::registerChamberAvailable: invalid ID %d", id);
	}
}

void GState::_registerChamberCompleted(int id) {
	if (isValidChamber(static_cast<ChamberID>(id))) {
		ChamberStats& stats = chamberStats.at(to_size_t(id));
		stats.timesCompleted = max<unsigned char>(stats.timesCompleted, 1);
	}
	else {
		log("GState::_registerChamberCompleted: invalid ID %d", id);
	}
}

bool GState::isChamberAvailable(ChamberID id)
{
	if (isValidChamber(id)) {
		return chambersAvailable.at(to_size_t(id));
	}
	else {
		log("GState::isChamberAvailable: invalid ID %d", id);
		return false;
	}
}

bool GState::isChamberCompleted(ChamberID id)
{
	if (isValidChamber(id)) {
		return chamberStats.at(to_size_t(id)).timesCompleted > 0;
	}
	else {
		log("GState::isChamberCompleted: invalid ID %d", id);
		return false;
	}
}

int GState::chambersCompletedCount()
{
	int result = 0;

	enum_foreach(ChamberID, id, begin, end) {
		if (isChamberCompleted(id))
			++result;
	}

	return result;
}

unsigned int GState::totalChamberTime()
{
	unsigned int result = 0;

	enum_foreach(ChamberID, id, begin, end) {
		result += chamberStats.at(to_size_t(id)).totalTimeMS;
	}

	return result;
}

void GState::_registerUpgrade(unsigned int at, unsigned int id)
{
	registerUpgrade(static_cast<Attribute>(at), id);
}

void GState::registerUpgrade(Attribute at, unsigned int id)
{
	upgrades.upgrades.at(to_size_t(at)) |= make_bitfield<AttributeSystem::upgradeCount>(id);
}

bool GState::isUpgradeAcquired(Attribute at, unsigned int id)
{
	if (id >= upgrades.upgrades.size()) {
		log("Invalid upgrade id %s, attribute %d", id, at);
		return false;
	}
	return upgrades.upgrades.at(to_size_t(at))[id];
}

float GState::getUpgradeLevel(Attribute at)
{
	return upgrades.upgrades.at(to_size_t(at)).count();
}

AttributeMap GState::getUpgrades()
{
	AttributeMap result;

	for (pair<Attribute, UpgradeInfo> entry : AttributeSystem::upgradeAttributes)
	{
		result.insert_or_assign(entry.first, entry.second.step * getUpgradeLevel(entry.first));
	}

	return result;
}

AttributeSystem GState::getPlayerStats()
{
	AttributeSystem result(FlandrePC::baseAttributes);
	result.apply(getUpgrades());
	return result;
}

void GState::setUpgradeLevels(int level)
{
	if (level >= 0)
	{
		registerUpgrade(Attribute::maxHP, 0);
		registerUpgrade(Attribute::shieldLevel, 0);
	}

	if (level >= 1)
	{
		registerUpgrade(Attribute::maxHP, 2);
	}

	if (level >= 2)
	{
		registerUpgrade(Attribute::maxMP, 1);
		registerUpgrade(Attribute::agility, 1);
		registerUpgrade(Attribute::ricochet, 1);
	}

	if (level >= 3)
	{
		registerUpgrade(Attribute::maxHP, 1);
	}

	if (level >= 4)
	{
		registerUpgrade(Attribute::attack, 1);
	}
}
