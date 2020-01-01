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
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "RumiaSpells.hpp"

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

void MarisaCollectMain::onEnter()
{
}

update_return MarisaCollectMain::update()
{
	GObject* player = getSpace()->getPlayer();
	
	autoUpdateFunction(moveFunction);

	if (player && !moveFunction) {
		moveFunction = ai::FollowPath::pathToTarget(fsm, player);
	}

	return_steady();
}

void MarisaForestMain::onEnter()
{
	gobject_ref player = fsm->getSpace()->getObjectRef("player");
	aimFunction = make_shared<ai::AimAtTarget>(fsm, player);
}

update_return MarisaForestMain::update()
{
	autoUpdateFunction(aimFunction);
	autoUpdateFunction(castFunction);

	if (!castFunction) {
		castFunction = fsm->make<ai::Cast>(Spell::getDescriptorByName("StarlightTyphoon"), -1.0);
		castFunction->onEnter();
	}

	return_steady();
}

ReimuYinYangOrbs::ReimuYinYangOrbs(StateMachine* fsm) :
	Function(fsm)
{
}

event_bitset ReimuYinYangOrbs::getEvents()
{
	return enum_bitfield2(event_type, detect, zeroHP);
}

bool ReimuYinYangOrbs::onEvent(Event event)
{
	if (event.isDetectPlayer() && !active) {
		Agent* agent = getAgent();
		auto props = app::getBullet("yinYangOrb");
		for_irange(i, 0, orbCount)
		{
			SpaceFloat angle = float_pi * (0.25 + i * 0.5);
			orbs[i] = agent->launchBullet(
				props,
				SpaceVect::ray(1.5, angle),
				angle,
				float_pi,
				false
			);
		}
		
		active = true;
		return true;
	}

	else if (event.eventType == event_type::zeroHP && active) {
		for_irange(i, 0, orbCount)
		{
			getSpace()->removeObject(orbs[i].get());
		}

		return true;
	}

	return false;
}

RumiaMain2::RumiaMain2(StateMachine* fsm, gobject_ref target) :
	Function(fsm),
	target(target)
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

void SakuyaMain::onEnter()
{
}

update_return SakuyaMain::update()
{
	return update_return{
		0,
		fsm->make<Cast>(Spell::getDescriptorByName("IllusionDial"), -1.0)
	};
}

const SpaceFloat IllusionDash::scale = 2.5;
const SpaceFloat IllusionDash::opacity = 0.25;
const SpaceFloat IllusionDash::speed = 10.0;

IllusionDash::IllusionDash(StateMachine* fsm, SpaceVect _target) :
	Function(fsm),
	target(_target)
{}

IllusionDash::IllusionDash(StateMachine* fsm, const ValueMap& args) :
	Function(fsm)
{
	auto t = args.at("target").asValueMap();

	target = SpaceVect(t.at("x").asFloat(), t.at("y").asFloat());
}

void IllusionDash::onEnter()
{
	GObject* agent = getObject();
	SpaceVect disp = displacementToTarget(agent, target);

	agent->setVel(disp.normalizeSafe()*speed);
	agent->addGraphicsAction(motionBlurStretch(
		disp.length() / speed,
		disp.toAngle(),
		opacity,
		scale
	));
}

update_return IllusionDash::update()
{
	GObject* agent = getObject();
	SpaceVect disp = displacementToTarget(agent, target);
	agent->setVel(disp.normalizeSafe()*speed);

	if (disp.lengthSq() < 0.125f) {
		agent->setVel(SpaceVect::zero);
		return_pop();
	}
	else {
		return_steady();
	}
}

}//end NS
