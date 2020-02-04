//
//  GState.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef GState_hpp
#define GState_hpp

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
		ar & mapFragments;
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

	unordered_map<string, int> attributes;
    unordered_map<string, unsigned int> itemRegistry;
	unordered_set<string> dialogs;
	unordered_set<string> objectRemovals;
	CharacterUpgrade upgrades;
	unordered_set<string> chambersAvailable;
	unordered_map<string, ChamberStats> chamberStats;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & attributes;
        ar & itemRegistry;
		ar & dialogs;
		ar & objectRemovals;
		ar & upgrades;
		ar & chambersAvailable;
		ar & chamberStats;
    }

	void addItem(string name);
	void addItem(string name, unsigned int count);
	bool hasItem(string name);
	unsigned int getItemCount(string name);
	bool removeItem(string name);
	bool removeItem(string name, unsigned int count);
	bool hasCompletedDialog(string name);
	void addObjectRemoval(string areaName, string objectName);
	bool isObjectRemoved(string areaName, string objectName);

	void registerChamberAvailable(string id);
	//Only used for testing
	void _registerChamberCompleted(string name);
	bool isChamberAvailable(string id);
	bool isChamberCompleted(string id);
	int chambersCompletedCount();
	unsigned int totalChamberTime();

	int getMapFragmentCount(string chamber);
	void registerMapFragment(string chamber, int mapID);

	void _registerUpgrade(unsigned int at, unsigned int id);
	void registerUpgrade(Attribute at, unsigned int id);
	bool isUpgradeAcquired(Attribute at, unsigned int id);
	float getUpgradeLevel(Attribute at);
	AttributeMap getUpgrades();
	AttributeSystem getPlayerStats();

	void checkInitAreaState(string name);

	void setAttribute(string name, int val);
	int getAttribute(string name);
	bool hasAttribute(string name);
	void incrementAttribute(string name);
	void subtractAttribute(string name, int val);

	//Apply all upgrades that are available at a certain point, for testing.
	void setUpgradeLevels(int level);
};

#endif /* GState_hpp */
