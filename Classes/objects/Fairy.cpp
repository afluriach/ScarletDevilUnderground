//
//  Fairy.cpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "Fairy.hpp"
#include "FirePattern.hpp"

const AttributeMap Fairy1::baseAttributes = {
	{Attribute::maxHP, 5.0f},
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 4.5f}
};

Fairy1::Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{}

void Fairy1::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 4.5f, 1.5f);
		}
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
	RegisterUpdate<Fairy2>(this)
{}


void Fairy2::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f);
		}
	));
	trackFunction = make_shared<ai::TrackByType<Fairy2>>();
	addThread(trackFunction);
}

void Fairy2::update()
{
	if (getHealth() / getMaxHealth() <= lowHealthRatio && !requestSent){
		trackFunction->messageTargets(&Fairy2::requestHandler, object_ref<Fairy2>(this));
		requestSent = true;
	}
}

void Fairy2::requestHandler(object_ref<Fairy2> other)
{
	gobject_ref player = space->getObjectRef("player");

	if (other.isValid() && player.isValid())
	{
		auto t = make_shared<ai::Thread>(
			make_shared<ai::OccupyMidpoint>(other.getBaseRef(), player),
			&fsm,
			playerShieldPriority,
			bitset<ai::lockCount>()
		);

		fsm.addThread(t);
	}
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
