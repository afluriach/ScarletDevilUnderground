//
//  EnemyFunctions.cpp
//  Koumachika
//
//  Created by Toni on 4/8/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"
#include "Graphics.h"
#include "MagicEffect.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "RumiaSpells.hpp"
#include "SpellDescriptor.hpp"

namespace ai{

const SpaceFloat BlueFairyPowerAttack::cooldown = 4.0;
const SpaceFloat BlueFairyPowerAttack::triggerLength = 1.0;
const SpaceFloat BlueFairyPowerAttack::triggerDist = 2.0;
const float BlueFairyPowerAttack::cost = 15.0f;

BlueFairyPowerAttack::BlueFairyPowerAttack(StateMachine* fsm) :
	Function(fsm)
{}

BlueFairyPowerAttack::~BlueFairyPowerAttack()
{
}

update_return BlueFairyPowerAttack::update()
{
	Agent* agent = getAgent();
	SpaceFloat targetDist = agent->getRadar()->getSensedObjectDistance(GType::player);
	auto& as = *agent->getAttributeSystem();

	if(!isSpellActive()) timerDecrement(timer);
	accumulator = 
		(accumulator + app::params.secondsPerFrame) * 
		to_int(!isnan(targetDist) && targetDist < triggerDist);

	if (
		!isSpellActive() &&
		as[Attribute::mp] >= cost &&
		accumulator >= triggerLength &&
		timer <= 0.0
	) {
		castSpell(Spell::getDescriptorByName("BlueFairyBomb"));
		accumulator = 0.0;
		timer = cooldown;
	}

	return_steady();
}

RumiaMain2::RumiaMain2(StateMachine* fsm, gobject_ref target) :
	Function(fsm),
	target(target)
{
}

RumiaMain2::~RumiaMain2()
{
}

void RumiaMain2::onEnter()
{
	flank = fsm->make<Flank>(target, 3.0, 1.0);
	flank->onEnter();

	dsd = fsm->make<RumiaDSD2>();
	dsd->onEnter();

	fire = fsm->make<FireAtTarget>(target);
	fire->onEnter();
}

update_return RumiaMain2::update()
{
	autoUpdateFunction(flank);
	autoUpdateFunction(fire);

	return_steady();
}

RumiaDSD2::~RumiaDSD2()
{
}

void RumiaDSD2::onEnter()
{
	castSpell( Spell::getDescriptorByName("DarknessSignDemarcation2"));
}

update_return RumiaDSD2::update()
{
	return_steady();
}

void RumiaDSD2::onExit()
{
	stopSpell();
}

}//end NS
