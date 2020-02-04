//
//  GState.cpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
//#include "GState.hpp"
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
	emplaceIfEmpty(itemRegistry, name, 0u);
	++itemRegistry.at(name);
}

void GState::addItem(string name, unsigned int count)
{
	emplaceIfEmpty(itemRegistry, name, 0u);
	itemRegistry.at(name) += count;
}

bool GState::removeItem(string name)
{
	return removeItem(name, 1);
}

bool GState::removeItem(string name, unsigned int count)
{
	auto it = itemRegistry.find(name);

	if (it == itemRegistry.end() || it->second < count) {
		return false;
	}
	else {
		it->second -= count;
		return true;
	}
}

bool GState::hasItem(string name)
{
	auto it = itemRegistry.find(name);
	return it != itemRegistry.end() && it->second != 0;
}

bool GState::hasCompletedDialog(string name)
{
	return dialogs.find(name) != dialogs.end();
}

void GState::registerChamberAvailable(string id)
{
	if (!id.empty()) {
		chambersAvailable.insert(id);
	}
}

void GState::_registerChamberCompleted(string name) {
	checkInitAreaState(name);
	unsigned char& completions = chamberStats.at(name).timesCompleted;
	completions = max<unsigned char>(completions, 1);
}

bool GState::isChamberAvailable(string id)
{
	return chambersAvailable.find(id) != chambersAvailable.end();
}

bool GState::isChamberCompleted(string id)
{
	auto it = chamberStats.find(id);
	return it != chamberStats.end() ? it->second.timesCompleted > 0 : false;
}

int GState::chambersCompletedCount()
{
	int result = 0;

	for (auto entry : chamberStats) {
		result += bool_int(entry.second.timesCompleted > 0);
	}

	return result;
}

unsigned int GState::totalChamberTime()
{
	unsigned int result = 0;

	for (auto entry : chamberStats) {
		result += entry.second.totalTimeMS;
	}

	return result;
}

int GState::getMapFragmentCount(string chamber)
{
	auto it = chamberStats.find(chamber);

	return it != chamberStats.end() ? it->second.mapFragments.count() : 0;
}

void GState::registerMapFragment(string chamber, int mapID)
{
	checkInitAreaState(chamber);

	if (mapID >= 0 && mapID < maxMapFragmentsPerChamber) {
		chamberStats.at(chamber).mapFragments.set(mapID);
	}
	else {
		log("GState::registerMapFragment: cannot register mapID %d!", mapID);
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
	AttributeSystem result(app::getAttributes(App::crntPC->attributes));
	result.apply(getUpgrades());
	return result;
}

void GState::checkInitAreaState(string name)
{
	emplaceIfEmpty(chamberStats, name);
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
	emplaceIfEmpty(attributes, name, 0);
	++attributes.at(name);
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

		addItem("PlayerBatMode");
	}

	if (level >= 2)
	{
		registerUpgrade(Attribute::maxMP, 1);
		registerUpgrade(Attribute::agility, 1);
		registerUpgrade(Attribute::bulletSpeed, 0);

		registerUpgrade(Attribute::attackSpeed, 1);
		registerUpgrade(Attribute::stamina, 1);

		addItem("ScarletDagger");
		addItem("Catadioptric");
	}

	if (level >= 3)
	{
		registerUpgrade(Attribute::shieldLevel, 1);

		addItem("StarbowBreak");
	}

	if (level >= 4)
	{
		registerUpgrade(Attribute::attackSpeed, 0);
		registerUpgrade(Attribute::bulletSpeed, 1);

		addItem("PlayerCounterClock");
	}
}
