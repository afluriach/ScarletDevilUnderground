//
//  Fairy.cpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "EnemyFirePattern.hpp"
#include "Fairy.hpp"
#include "value_map.hpp"

const AttributeMap Fairy1::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 4.5f}
};

Fairy1::Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1)
{}

void Fairy1::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 4.5f, 1.5f);
		}
	));
}

const AttributeMap Fairy1A::baseAttributes = {
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

Fairy1A::Fairy1A(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1)
{}

void Fairy1A::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::LookAround>(float_pi / 4.0));
	addThread(make_shared<ai::FireIfTargetVisible>(
		make_shared<Fairy1ABulletPattern>(this),
		sm.agent->space->getObjectRef("player")
	));
}

const AttributeMap Fairy1B::baseAttributes = {
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, .1f },
	{ Attribute::acceleration, 4.5f }
};

Fairy1B::Fairy1B(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power1),
	waypointName(getStringOrDefault(args, "waypoint", ""))
{}

void Fairy1B::initStateMachine(ai::StateMachine& sm) {
	if (!waypointName.empty()){
		SpaceVect waypoint = space->getWaypoint(waypointName);
		SpaceFloat angularPos = ai::directionToTarget(this, waypoint).toAngle() + float_pi;
		addThread(make_shared<ai::CircleAround>(waypoint, angularPos, float_pi / 4.0));
	}

	addThread(make_shared<ai::FireIfTargetVisible>(
		make_shared<Fairy1ABulletPattern>(this),
		sm.agent->space->getObjectRef("player")
	));
}

const AttributeMap Fairy2::baseAttributes = {
	{Attribute::maxHP, 15.0f},
	{Attribute::speed, 4.5f},
	{Attribute::acceleration, 4.5f}
};

const boost::rational<int> Fairy2::lowHealthRatio = boost::rational<int>(1, 3);

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::power2),
	RegisterUpdate<Fairy2>(this)
{}


void Fairy2::initStateMachine(ai::StateMachine& sm) {
	auto detectMain = make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f);
		}
	);

	auto detectThread = make_shared<ai::Thread>(
		detectMain,
		&fsm,
		to_int(ai_priority::engage),
		bitset<ai::lockCount>()
	);

	fsm.addThread(detectThread);
}

void Fairy2::addFleeThread()
{
	GObject* player = space->getObject("player");
	auto fleeThread = make_shared<ai::Thread>(
		make_shared<ai::Flee>(player, 5.0f),
		&fsm,
		to_int(ai_priority::flee),
		bitset<ai::lockCount>()
		);
	fsm.addThread(fleeThread);
}

void Fairy2::addSupportThread(object_ref<Fairy2> other)
{
	gobject_ref player = space->getObjectRef("player");

	if (!player.isValid()) {
		return;
	}

	auto t = make_shared<ai::Thread>(
		make_shared<ai::OccupyMidpoint>(other.getBaseRef(), player),
		&fsm,
		to_int(ai_priority::support),
		bitset<ai::lockCount>()
	);
	supportThread = t->uuid;

	fsm.addThread(t);
}

void Fairy2::removeSupportThread()
{
	if (supportThread != 0) {
		fsm.removeThread(supportThread);
		supportThread = 0;
	}
}

void Fairy2::update()
{
	if (getHealth() / getMaxHealth() <= lowHealthRatio && (crntState == ai_state::normal || crntState == ai_state::supporting)){
		space->messageAll(this, &Fairy2::requestHandler, &Fairy2::responseHandler, object_ref<Fairy2>(this));

		crntState = ai_state::flee;
		addFleeThread();
	}
}

object_ref<Fairy2> Fairy2::requestHandler(object_ref<Fairy2> other)
{
	if (other.isValid() && crntState == ai_state::normal) {
		crntState = ai_state::supportOffered;
		return this;
	}
	else {
		return nullptr;
	}
}

void Fairy2::responseHandler(object_ref<Fairy2> supporting)
{
	if (supporting.isValid() && crntState == ai_state::flee)
	{
		crntState = ai_state::fleeWithSupport;
		supporting.get()->message(supporting.get(), &Fairy2::acknowledgeHandaler, object_ref<Fairy2>(this));
	}
}

void Fairy2::acknowledgeHandaler(object_ref<Fairy2> supportTarget)
{
	crntState = ai_state::supporting;
	addSupportThread(supportTarget);
}

const AttributeMap IceFairy::baseAttributes = {
	{Attribute::maxHP, 15.0f},
	{Attribute::speed, 4.5f},
	{Attribute::acceleration, 4.5f},
	{Attribute::iceSensitivity, 0.0f}
};

void IceFairy::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
		return make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f);
	}
	));

	Agent *const _agent = sm.getAgent();

	addThread(make_shared<ai::Detect>(
		"player",
		[_agent](GObject* target) -> shared_ptr<ai::FireAtTarget> {
		return make_shared<ai::FireAtTarget>(make_shared<IceFairyBulletPattern>(_agent), target);
	}
	));

}
