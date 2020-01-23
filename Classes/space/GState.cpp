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

void GState::registerChamberAvailable(ChamberID id)
{
	if (isValidChamber(id)) {
		chambersAvailable.at(to_size_t(id)) = true;
	}
	else {
		log("GState::registerChamberAvailable: invalid ID %d", to_int(id));
	}
}

void GState::_registerChamberCompleted(int id) {
	if (isValidChamber(static_cast<ChamberID>(id))) {
		ChamberStats& stats = chamberStats.at(to_size_t(id));
		stats.timesCompleted = max<unsigned char>(stats.timesCompleted, 1);
	}
	else {
		log("GState::_registerChamberCompleted: invalid ID %d", to_int(id));
	}
}

bool GState::isChamberAvailable(ChamberID id)
{
	if (isValidChamber(id)) {
		return chambersAvailable.at(to_size_t(id));
	}
	else {
		log("GState::isChamberAvailable: invalid ID %d", to_int(id));
		return false;
	}
}

bool GState::isChamberCompleted(ChamberID id)
{
	if (isValidChamber(id)) {
		return chamberStats.at(to_size_t(id)).timesCompleted > 0;
	}
	else {
		log("GState::isChamberCompleted: invalid ID %d", to_int(id));
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

int GState::getMapFragmentCount(ChamberID chamber)
{
	return chamberStats.at(to_size_t(chamber)).mapFragments.count();
}

void GState::registerMapFragment(ChamberID chamber, int mapID)
{
	if (mapID >= 0 && mapID < maxMapFragmentsPerChamber) {
		chamberStats.at(to_size_t(chamber)).mapFragments.set(mapID);
	}
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
		log("Invalid upgrade id %s, attribute %d", to_int(id), to_int(at));
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
	AttributeSystem result(app::getAttributes(FlandrePC::baseAttributes));
	result.apply(getUpgrades());
	return result;
}

void GState::setAttribute(string name, int val)
{
	attributes.insert_or_assign(name, val);
}

int GState::getAttribute(string name)
{
	return getOrDefault(attributes, name, 0);
}

bool GState::hasAttribute(string name)
{
	return attributes.find(name) != attributes.end();
}

void GState::incrementAttribute(string name)
{
	auto it = attributes.find(name);

	if(it != attributes.end()){
		++it->second;
	}
	else {
		attributes.insert_or_assign(name, 1);
	}
}

void GState::subtractAttribute(string name, int val)
{
	auto it = attributes.find(name);

	if (it != attributes.end()) {
		it->second -= val;
	}
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
		registerUpgrade(Attribute::maxStamina, 0);

		itemRegistry.insert("PlayerBatMode");
	}

	if (level >= 2)
	{
		registerUpgrade(Attribute::maxMP, 1);
		registerUpgrade(Attribute::agility, 1);
		registerUpgrade(Attribute::bulletSpeed, 0);

		registerUpgrade(Attribute::attackSpeed, 1);
		registerUpgrade(Attribute::stamina, 1);

		itemRegistry.insert("ScarletDagger");
		itemRegistry.insert("Catadioptric");
	}

	if (level >= 3)
	{
		registerUpgrade(Attribute::shieldLevel, 1);

		itemRegistry.insert("StarbowBreak");
	}

	if (level >= 4)
	{
		registerUpgrade(Attribute::attackSpeed, 0);
		registerUpgrade(Attribute::bulletSpeed, 1);

		itemRegistry.insert("PlayerCounterClock");
	}
}
