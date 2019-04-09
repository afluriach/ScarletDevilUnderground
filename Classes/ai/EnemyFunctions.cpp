//
//  EnemyFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/8/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"
#include "macros.h"

namespace ai{

const SpaceFloat BlueFairyPowerAttack::cooldown = 4.0;
const SpaceFloat BlueFairyPowerAttack::triggerLength = 1.0;
const SpaceFloat BlueFairyPowerAttack::triggerDist = 2.0;
const float BlueFairyPowerAttack::cost = 15.0f;

BlueFairyPowerAttack::BlueFairyPowerAttack() {}

void BlueFairyPowerAttack::update(StateMachine& fsm)
{
	SpaceFloat targetDist = fsm.getAgent()->getSensedObjectDistance(GType::player);
	auto& as = *fsm.getAgent()->getAttributeSystem();

	if(!fsm.agent->isSpellActive()) timerDecrement(timer);
	accumulator = (accumulator + App::secondsPerFrame)*to_int(!isnan(targetDist) && targetDist < triggerDist);

	if (
		!fsm.agent->isSpellActive() &&
		as[Attribute::mp] >= cost &&
		accumulator >= triggerLength &&
		timer <= 0.0
	) {
		fsm.agent->cast(make_shared<BlueFairyBomb>(fsm.agent));
		accumulator = 0.0;
		timer = cooldown;
	}
}

}//end NS
