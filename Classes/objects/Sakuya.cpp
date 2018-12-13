//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "Prefix.h"

#include "GSpace.hpp"
#include "Sakuya.hpp"
#include "scenes.h"

const AttributeMap Sakuya::baseAttributes = boost::assign::map_list_of
	(Attribute::maxHP, 30.0f)
	(Attribute::speed, 3.0f)
	(Attribute::acceleration, 4.5f)
;

void Sakuya::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<ai::SakuyaMain>());
}
