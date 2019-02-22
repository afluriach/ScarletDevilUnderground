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
	Enemy(collectible_id::magic2)
{}

void ReimuEnemy::initStateMachine(ai::StateMachine& fsm)
{
	fsm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			if(!sm.isThreadRunning("ReimuMain"))
				sm.addThread(make_shared<ReimuMain>());
		}
	);
}

void ReimuMain::onEnter(ai::StateMachine& sm)
{
	sm.agent->space->createObject(GObject::make_object_factory<YinOrb>(sm.getAgent(), sm.agent->getPos() + SpaceVect(1.0, 0.0)));
	sm.agent->space->createObject(GObject::make_object_factory<YangOrb>(sm.getAgent(), sm.agent->getPos() + SpaceVect(-1.0, 0.0)));
}

void ReimuMain::update(ai::StateMachine& sm)
{

}

