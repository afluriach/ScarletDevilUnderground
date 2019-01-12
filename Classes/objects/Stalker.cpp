//
//  Stalker.cpp
//  Koumachika
//
//  Created by Toni on 12/12/17.
//
//

#include "Prefix.h"

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
	Enemy(collectible_id::magic1),
	RegisterUpdate<Stalker>(this)
{}

void Stalker::initStateMachine(ai::StateMachine& sm)
{
	auto t1 = make_shared<ai::Thread>(
		make_shared<StalkerMain>(),
		&sm,
		1,
		bitset<ai::lockCount>()
	);

	auto t2 = make_shared<ai::Thread>(
		make_shared<ai::DetectAndSeekPlayer>(),
		&sm,
		0,
		bitset<ai::lockCount>()
	);

	sm.addThread(t1);
	sm.addThread(t2);

	attributeSystem.setFullStamina();
}

void Stalker::update()
{
	SpaceFloat dp = getVel().length() * App::secondsPerFrame;
	modifyAttribute(Attribute::stamina, -dp);
}

void Stalker::teleport(SpaceVect pos)
{
	attributeSystem.setFullStamina();
	
	GObject::teleport(pos);
}


void StalkerMain::onEnter(ai::StateMachine& sm)
{
}

void StalkerMain::update(ai::StateMachine& sm)
{
	if (sm.getAgent()->getAttribute(Attribute::stamina) <= 0.0f) {
		sm.push(make_shared<StalkerTeleport>());
	}
}

StalkerTeleport::StalkerTeleport() :
ai::Cast(make_spell_generator<Teleport>())
{
}
