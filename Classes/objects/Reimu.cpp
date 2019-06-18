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
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

const string ReimuEnemy::baseAttributes = "reimu";
const string ReimuEnemy::properName = "Reimu Hakurei";

ReimuEnemy::ReimuEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
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
		ActivateableObject* ao = space->getObjectAs<ActivateableObject>(name);
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
		ActivateableObject* ao = space->getObjectAs<ActivateableObject>(name);
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
		orbs[i] = space->createObject(GObject::make_object_factory<YinYangOrb>(getPos() + SpaceVect::ray(1.5, angle), angle, this));
	}
}

void ReimuEnemy::removeOrbs()
{
	for_irange(i, 0, orbCount)
	{
		space->removeObject(orbs[i].get());
	}
}

void ReimuEnemy::initStateMachine()
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("ReimuMain")) {
				space->createDialog("dialogs/reimu_forest_pre_fight", false);
				this->lockDoors();
				this->spawnOrbs();
				fsm.addThread(make_shared<ReimuMain>(&fsm));
				fsm.addThread(make_shared<ai::FireAtTarget>(&fsm, target));
				fsm.addThread(make_shared<ai::Flank>(&fsm, target, 3.0, 2.0), 1);
			}
		}
	);
}

void ReimuMain::onEnter()
{
}

ai::update_return ReimuMain::update()
{
	return_steady();
}

void ReimuMain::onExit()
{
}
