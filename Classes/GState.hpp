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
#include "types.h"

struct CharacterUpgrade
{
	array<bitset<AttributeSystem::upgradeCount>, AttributeSystem::upgradeAttributesCount> upgrades;

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

	static const unsigned int version = 1;
	static const unsigned int maxProfiles = 9;

	static vector<bool> profileSlotsInUse;

	static void initProfiles();

    set<string> itemRegistry;
	array<CharacterUpgrade, to_size_t(PlayerCharacter::end)> upgrades;
	array<bool, to_size_t(ChamberID::end)> chambersAvailable;

	unsigned char mushroomFlags = 0;
	unsigned char mushroomCount = 0;
	unsigned char blueFairies = 0;

    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & itemRegistry;
		ar & upgrades;
		ar & chambersAvailable;
		ar & mushroomFlags;
		ar & mushroomCount;
		ar & blueFairies;
    }

	bool hasItem(string name);

	void registerMushroomAcquired(int id);
	bool isMushroomAcquired(int id);

	void setBlueFairyLevel(int id);
	int getBlueFairyLevel();

	void registerChamberAvailable(ChamberID id);
	bool isChamberAvailable(ChamberID id);

	void registerUpgrade(PlayerCharacter pc, Attribute at, unsigned int id);
	bool isUpgradeAcquired(PlayerCharacter pc, Attribute at, unsigned int id);
	float getUpgradeLevel(PlayerCharacter pc, Attribute at);
};

#endif /* GState_hpp */
