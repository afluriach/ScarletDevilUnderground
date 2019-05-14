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

const AttributeMap ReimuEnemy::baseAttributes = {
	{ Attribute::maxHP, 100.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::touchDamage, 10.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};
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

void ReimuEnemy::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("ReimuMain")) {
				sm.agent->space->createDialog("dialogs/reimu_forest_pre_fight", false);
				this->lockDoors();
				this->spawnOrbs();
				sm.addThread(make_shared<ReimuMain>());
				sm.addThread(make_shared<ai::FireAtTarget>(target));
				sm.addThread(make_shared<ai::Flank>(target, 3.0, 2.0), 1);
			}
		}
	);
}

void ReimuMain::onEnter(ai::StateMachine& sm)
{
}

void ReimuMain::update(ai::StateMachine& sm)
{

}

void ReimuMain::onExit(ai::StateMachine& sm)
{
}
