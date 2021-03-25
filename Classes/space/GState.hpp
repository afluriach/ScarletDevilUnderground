//
//  GState.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef GState_hpp
#define GState_hpp

struct AreaStats
{
	unsigned long totalTime = 0;

	int timesCleared = 0;
	int enemiesDefeated = 0;

	rooms_bitmask roomsVisited;
	rooms_bitmask roomsMapped;

	set<string> objectRemovals;

	template<class Archive>
	void serialize(Archive& ar, const unsigned int version)
	{
		ar & totalTime;		
		ar & timesCleared;
		
		ar & roomsVisited;
		ar & roomsMapped;

		ar & objectRemovals;
	}

	void addObjectRemoval(string objectName);
	bool isObjectRemoved(string objectName) const;
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

	map<string, AreaStats> areaStats;

	map<string, int> attributes;
	map<Attribute, float> attributeUpgrades;
	map<string, unsigned int> itemRegistry;
	map<string, unsigned int> enemiesDefeated;

	unordered_set<string> dialogs;

	unsigned long saveCount = 0;
	unsigned long elapsedTime = 0;
	unsigned int totalEnemiesDefeated = 0;

	string area;
	string entrance;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
		ar & areaStats;

		ar & attributes;
		ar & attributeUpgrades;
        ar & itemRegistry;
		ar & enemiesDefeated;

		ar & dialogs;

		ar & saveCount;
		ar & elapsedTime;
		ar & totalEnemiesDefeated;

		ar & area;
		ar & entrance;
    }

	void addItem(string name);
	void addItem(string name, unsigned int count);
	bool hasItem(string name);
	unsigned int getItemCount(string name);
	bool removeItem(string name);
	bool removeItem(string name, unsigned int count);
	bool hasCompletedDialog(string name);
	void applyAttributeUpgrade(Attribute attr, float val);

	bool hasClearedArea(string id);
	unsigned long getAreaTime(string id);
	int areasClearedCount();

	AttributeSystem getPlayerStats();
	unsigned long getElapsedTime();
	void incrementElapsedTime();
	void incrementAreaTime(string areaID);

	void checkInitAreaState(string name);

	void setAttribute(string name, int val);
	int getAttribute(string name);
	bool hasAttribute(string name);
	void incrementAttribute(string name);
	void subtractAttribute(string name, int val);
};

#endif /* GState_hpp */
