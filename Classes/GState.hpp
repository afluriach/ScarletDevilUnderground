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
    static const unsigned int version = 1;
    
    friend class boost::serialization::access;

    set<string> itemRegistry;
	array<CharacterUpgrade, to_size_t(PlayerCharacter::end)> upgrades;
    
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & itemRegistry;
		ar & upgrades;
    }

	void registerUpgrade(PlayerCharacter pc, Attribute at, unsigned int id);
	bool isUpgradeAcquired(PlayerCharacter pc, Attribute at, unsigned int id);
	float getUpgradeLevel(PlayerCharacter pc, Attribute at);
};

#endif /* GState_hpp */
