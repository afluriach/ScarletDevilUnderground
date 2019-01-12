//
//  Upgrade.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Upgrade.hpp"

Upgrade::Upgrade(unsigned int id, Attribute attribute) :
attribute(attribute),
upgrade_id(id),
RegisterInit<Upgrade>(this)
{}

void Upgrade::init()
{
	if (App::crntState->isUpgradeAcquired(App::crntPC, attribute, upgrade_id)) {
		space->removeObject(this);
	}
}

const Attribute HPUpgrade::attribute = Attribute::maxHP;
const string HPUpgrade::spriteName = "hp_upgrade";

HPUpgrade::HPUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(UpgradeImpl<HPUpgrade>)
{}

const Attribute MPUpgrade::attribute = Attribute::maxMP;
const string MPUpgrade::spriteName = "mp_upgrade";

MPUpgrade::MPUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(UpgradeImpl<MPUpgrade>)
{}

const Attribute PowerUpgrade::attribute = Attribute::maxPower;
const string PowerUpgrade::spriteName = "power_upgrade";

PowerUpgrade::PowerUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(UpgradeImpl<PowerUpgrade>)
{}

const Attribute AgilityUpgrade::attribute = Attribute::agility;
const string AgilityUpgrade::spriteName = "agility_upgrade";

AgilityUpgrade::AgilityUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(UpgradeImpl<AgilityUpgrade>)
{}
