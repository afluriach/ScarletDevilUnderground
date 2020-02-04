//
//  Upgrade.cpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#include "Prefix.h"

#include "Player.hpp"
#include "Upgrade.hpp"
#include "value_map.hpp"

bool Upgrade::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int upgradeID = getIntOrDefault(args, "id", -1);
	Attribute attr = AttributeSystem::getAttribute(getStringOrDefault(args, "attr", ""));

	return (
		upgradeID != -1 &&
		attr != Attribute::end &&
		!App::crntState->isUpgradeAcquired(attr, upgradeID)
	);
}

Upgrade::Upgrade(GSpace* space, ObjectIDType id, const ValueMap& args) :
	GObject(MapParamsPointUp(), physics_params(GType::playerPickup, PhysicsLayers::ground, 0.75, -1.0, true)),
	attribute(AttributeSystem::getAttribute(getStringOrDefault(args, "attr", ""))),
	upgrade_id(getIntOrDefault(args, "id", -1))
{
	if (attribute == Attribute::end || upgrade_id == -1) {
		log("Upgrade %s: invalid params", getName());
		space->removeObject(this);
	}
}

string Upgrade::getSprite() const
{
	return AttributeSystem::upgradeAttributes.at(attribute).sprite;
}

string Upgrade::itemName() const
{
	return "";
}

void Upgrade::onPlayerContact(Player* p) {
	onAcquire(p);
}

void Upgrade::onAcquire(Player* player)
{
	player->applyUpgrade(this);
}
