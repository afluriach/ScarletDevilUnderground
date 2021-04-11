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

void AreaStats::addObjectRemoval(string objectName)
{
	objectRemovals.insert(objectName);
}

bool AreaStats::isObjectRemoved(string objectName) const
{
	auto it = objectRemovals.find(objectName);
	return it != objectRemovals.end();
}

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

void GState::applyAttributeUpgrade(Attribute attr, float val)
{
	emplaceIfEmpty(attributeUpgrades, attr, 0.0f);

	attributeUpgrades.at(attr) += val;
}

bool GState::hasClearedArea(string id)
{
	auto it = areaStats.find(id);
	return it != areaStats.end() ? it->second.timesCleared > 0 : false;
}

unsigned long GState::getElapsedTime()
{
	return elapsedTime;
}

void GState::incrementElapsedTime()
{
	++elapsedTime;
}

void GState::incrementAreaTime(string areaID)
{
	auto it = areaStats.find(areaID);
	if (it != areaStats.end()) {
		++it->second.totalTime;
	}
}

int GState::areasClearedCount()
{
	int clears = 0;

	for (auto entry : areaStats)
	{
		if (entry.second.timesCleared > 0)
			++clears;
	}

	return clears;
}

AttributeSystem GState::getPlayerStats()
{
	AttributeSystem result(GSpace::playerCharacter->attributes);
	result.apply(attributeUpgrades);
	return result;
}

void GState::checkInitAreaState(string name)
{
	emplaceIfEmpty(areaStats, name);
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

void GState::registerEnemyDefeated(string t)
{
    ++totalEnemiesDefeated;

    emplaceIfEmpty(enemiesDefeated, t, to_uint(0));
    ++enemiesDefeated.at(t);
}
