//
//  Reimu.cpp
//  Koumachika
//
//  Created by Toni on 2/22/19.
//
//

#include "Prefix.h"

#include "EnemyBullet.hpp"
#include "Reimu.hpp"
#include "value_map.hpp"

Reimu::Reimu(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

const AttributeMap ReimuEnemy::baseAttributes = {
	{ Attribute::maxHP, 100.0f },
	{ Attribute::maxMP, 5.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 4.0f }
};

ReimuEnemy::ReimuEnemy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2),
	activations(getStringOrDefault(args, "activations", ""))
{}

void ReimuEnemy::onZeroHP()
{
	space->createDialog("dialogs/reimu_forest_post_fight", false);
	unlockDoors();
	if (yin.isValid()) space->removeObject(yin.get());
	if (yang.isValid()) space->removeObject(yang.get());
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

void ReimuEnemy::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("ReimuMain")) {
				sm.agent->space->createDialog("dialogs/reimu_forest_pre_fight", false);
				this->lockDoors();
				yin = space->createObject(GObject::make_object_factory<YinOrb>(sm.getAgent(), sm.agent->getPos() + SpaceVect(1.0, 0.0)));
				yang = space->createObject(GObject::make_object_factory<YangOrb>(sm.getAgent(), sm.agent->getPos() + SpaceVect(-1.0, 0.0)));
				sm.addThread(make_shared<ReimuMain>());
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
