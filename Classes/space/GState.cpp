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

unsigned int GState::getItemCount(string name)
{
	auto it = itemRegistry.find(name);
	return it != itemRegistry.end() ? it->second : 0;
}

bool GState::hasCompletedDialog(string name)
{
	return dialogs.find(name) != dialogs.end();
}

void GState::addObjectRemoval(string areaName, string objectName)
{
	if (areaName.size() > 0 && objectName.size() > 0) {
		objectRemovals.insert(areaName + "." + objectName);
	}
	else {
		log("addObjectRemoval: invalid parameters!");
	}
}

bool GState::isObjectRemoved(string areaName, string objectName)
{
	if (areaName.size() > 0 && objectName.size() > 0) {
		string key = areaName + "." + objectName;
		return objectRemovals.find(key) != objectRemovals.end();
	}
	else {
		log("isObjectRemoved: invalid parameters!");
		return false;
	}
}

void GState::applyAttributeUpgrade(Attribute attr, float val)
{
	emplaceIfEmpty(attributeUpgrades, attr, 0.0f);

	attributeUpgrades.at(attr) += val;
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

AttributeSystem GState::getPlayerStats()
{
	AttributeSystem result(app::getAttributes(App::crntPC->attributes));
	result.apply(attributeUpgrades);
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
