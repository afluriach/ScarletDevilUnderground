//
//  GState.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef GState_hpp
#define GState_hpp

#include "Attributes.hpp"
#include "enum.h"

struct ChamberStats
{
	unsigned int totalTimeMS = 0;
	unsigned int fastestTimeMS = 0;

	unsigned char timesCompleted = 0;
	unsigned char maxEnemiesDefeated = 0;

	rooms_bitmask roomsVisited;
	map_fragments_bitmask mapFragments;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & totalTimeMS;
		ar & fastestTimeMS;
		ar & timesCompleted;
		ar & maxEnemiesDefeated;
		ar & roomsVisited;
	}
};

struct CharacterUpgrade
{
	array<bitset<AttributeSystem::upgradeCount>, to_size_t(Attribute::end)> upgrades = {};

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & upgrades;
	}
};

//All of the persistent state associated with a single profile.
class GState
{
public:    
    friend class boost::serialization::access;

	static constexpr unsigned int version = 1;
	static constexpr unsigned int maxProfiles = 9;

	static vector<bool> profileSlotsInUse;

	static void initProfiles();

    set<string> itemRegistry;
	CharacterUpgrade upgrades;
	array<bool, to_size_t(ChamberID::end)> chambersAvailable = {};
	array<ChamberStats, to_size_t(ChamberID::end)> chamberStats = {};

	unsigned char mushroomFlags = 0;
	unsigned char mushroomCount = 0;
	unsigned char blueFairies = 0;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & itemRegistry;
		ar & upgrades;
		ar & chambersAvailable;
		ar & chamberStats;

		ar & mushroomFlags;
		ar & mushroomCount;
		ar & blueFairies;
    }

	void addItem(string name);
	bool hasItem(string name);

	void registerMushroomAcquired(int id);
	bool isMushroomAcquired(int id);

	void setBlueFairyLevel(int id);
	int getBlueFairyLevel();

	void registerChamberAvailable(ChamberID id);
	//Only used for testing
	void _registerChamberCompleted(int id);
	bool isChamberAvailable(ChamberID id);
	bool isChamberCompleted(ChamberID id);
	int chambersCompletedCount();
	unsigned int totalChamberTime();

	int getMapFragmentCount(ChamberID chamber);
	void registerMapFragment(ChamberID chamber, int mapID);

	void _registerUpgrade(unsigned int at, unsigned int id);
	void registerUpgrade(Attribute at, unsigned int id);
	bool isUpgradeAcquired(Attribute at, unsigned int id);
	float getUpgradeLevel(Attribute at);
	AttributeMap getUpgrades();
	AttributeSystem getPlayerStats();

	//Apply all upgrades that are available at a certain point, for testing.
	void setUpgradeLevels(int level);
};

#endif /* GState_hpp */
