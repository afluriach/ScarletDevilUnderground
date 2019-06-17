//
//  EnemyFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/8/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"

namespace ai{

const SpaceFloat BlueFairyPowerAttack::cooldown = 4.0;
const SpaceFloat BlueFairyPowerAttack::triggerLength = 1.0;
const SpaceFloat BlueFairyPowerAttack::triggerDist = 2.0;
const float BlueFairyPowerAttack::cost = 15.0f;

BlueFairyPowerAttack::BlueFairyPowerAttack(StateMachine* fsm) :
	Function(fsm)
{}

update_return BlueFairyPowerAttack::update()
{
	SpaceFloat targetDist = agent->getSensedObjectDistance(GType::player);
	auto& as = *agent->getAttributeSystem();

	if(!agent->isSpellActive()) timerDecrement(timer);
	accumulator = 
		(accumulator + app::params.secondsPerFrame) * 
		to_int(!isnan(targetDist) && targetDist < triggerDist);

	if (
		!agent->isSpellActive() &&
		as[Attribute::mp] >= cost &&
		accumulator >= triggerLength &&
		timer <= 0.0
	) {
		agent->cast(make_shared<BlueFairyBomb>(agent));
		accumulator = 0.0;
		timer = cooldown;
	}

	return_steady();
}

}//end NS
