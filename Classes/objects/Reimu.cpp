//
//  Reimu.cpp
//  Koumachika
//
//  Created by Toni on 2/22/19.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "EnemyBullet.hpp"
#include "EnemyFirePattern.hpp"
#include "GSpace.hpp"
#include "Reimu.hpp"
#include "value_map.hpp"

Reimu::Reimu(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent)
{}

const string ReimuEnemy::baseAttributes = "reimu";
const string ReimuEnemy::properName = "Reimu Hakurei";

ReimuEnemy::ReimuEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2),
	activations(getStringOrDefault(args, "activations", ""))
{
	firePattern = make_shared<ReimuWavePattern>(this);
}

void ReimuEnemy::onZeroHP()
{
	space->createDialog("dialogs/reimu_forest_post_fight", false);
	unlockDoors();
	removeOrbs();
	Agent::onZeroHP();
}

void ReimuEnemy::lockDoors()
{
	vector<string> names = splitString(activations, " ");

	for (string name : names)
	{
		GObject* ao = space->getObject(name);
		if (ao) {
			ao->activate();
		}
	}
}

void ReimuEnemy::unlockDoors()
{
	vector<string> names = splitString(activations, " ");

	for (string name : names)
	{
		GObject* ao = space->getObject(name);
		if (ao) {
			ao->deactivate();
		}
	}
}

void ReimuEnemy::spawnOrbs()
{
	for_irange(i, 0, orbCount)
	{
		SpaceFloat angle = float_pi * (0.25 + i*0.5);
		orbs[i] = space->createObject<YinYangOrb>(
			getPos() + SpaceVect::ray(1.5, angle),
			angle,
			getBulletAttributes(app::getBullet(YinYangOrb::props))
		);
	}
}

void ReimuEnemy::removeOrbs()
{
	for_irange(i, 0, orbCount)
	{
		space->removeObject(orbs[i].get());
	}
}
