//
//  Upgrade.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "GState.hpp"
#include "Upgrade.hpp"

Upgrade::Upgrade(GSpace* space, ObjectIDType id, const ValueMap& args, Attribute at) :
	MapObjForwarding(GObject),
	attribute(at),
	upgrade_id(getInt(args, "id"))
{
	setInitialAngle(float_pi / 2.0);
}

string Upgrade::imageSpritePath() const
{
	return AttributeSystem::upgradeAttributes.at(attribute).sprite;
}
