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

const AttributeMap Fairy1::baseAttributes = boost::assign::map_list_of
(Attribute::maxHP, 5.0f)
(Attribute::speed, 3.0f)
(Attribute::acceleration, 4.5f)
;

void Fairy1::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 4.5f, 1.5f);
		}
	));
}

const AttributeMap Fairy2::baseAttributes = boost::assign::map_list_of
(Attribute::maxHP, 15.0f)
(Attribute::speed, 4.5f)
(Attribute::acceleration, 4.5f)
;

void Fairy2::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::Detect>(
		"player",
		[](GObject* target) -> shared_ptr<ai::Function> {
			return make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f);
		}
	));
}

const AttributeMap IceFairy::baseAttributes = boost::assign::map_list_of
(Attribute::maxHP, 15.0f)
(Attribute::speed, 4.5f)
(Attribute::acceleration, 4.5f)
(Attribute::iceSensitivity, 0.0f)
;

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
