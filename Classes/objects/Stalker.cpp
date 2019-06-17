//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "EnemySpell.hpp"
#include "Stalker.hpp"
#include "TeleportPad.hpp"

const AttributeMap Stalker::baseAttributes = {
	{Attribute::maxHP, 12.0f},
	{Attribute::maxStamina, 10.0f},
	{Attribute::speed, 1.5f},
	{Attribute::acceleration, 4.5f}
};

Stalker::Stalker(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic1)
{}

void Stalker::initStateMachine()
{
	auto t1 = make_shared<ai::Thread>(
		make_shared<StalkerMain>(&fsm),
		&fsm,
		1,
		bitset<ai::lockCount>()
	);
	fsm.addThread(t1);

	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Seek>(&fsm, target, true));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Seek");
		}
	);

	attributeSystem.setFullStamina();
}

void Stalker::update()
{
	Enemy::update();

	SpaceFloat dp = getVel().length() * app::params.secondsPerFrame;
	modifyAttribute(Attribute::stamina, -dp);
}

void Stalker::teleport(SpaceVect pos)
{
	attributeSystem.setFullStamina();
	
	GObject::teleport(pos);
}


void StalkerMain::onEnter()
{
}

ai::update_return StalkerMain::update()
{
	return_push_if_true(
		agent->getAttribute(Attribute::stamina) <= 0.0f,
		fsm->make<ai::Cast>(make_spell_generator<Teleport>())
	);
}
