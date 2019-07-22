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
#include "GSpace.hpp"
#include "MarisaSpell.hpp"
#include "Player.hpp"
#include "RadarSensor.hpp"
#include "RumiaSpells.hpp"
#include "SakuyaSpell.hpp"

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
	SpaceFloat targetDist = agent->getRadar()->getSensedObjectDistance(GType::player);
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

void BatMain::onEnter()
{
	target = getSpace()->getPlayer();
}

update_return BatMain::update()
{
	if (target.isValid())
		return_push(fsm->make<Flank>(target, 2.0, 1.0));
	else
		return_pop();
}

void FacerMain::onEnter()
{
	target = getSpace()->getPlayer();
}

update_return FacerMain::update()
{
	if (target.isValid()) {
		if (isFacingTarget(agent, target.get())) {
			agent->setVel(SpaceVect::ray(agent->getMaxSpeed(), agent->getAngle()));
		}
		else {
			agent->setVel(SpaceVect::zero);
		}
	}

	return_pop_if_false(target.isValid());
}

bool FacerMain::onEvent(Event event)
{
	bool handle = event.eventType == event_type::bulletHit;

	if (handle) {
		agent->rotate(float_pi * 0.5);
	}

	return handle;
}

void FollowerMain::onEnter()
{
	target = getSpace()->getPlayer();
}

update_return FollowerMain::update()
{
	if (target.isValid()) {
		if (isFacingTargetsBack(agent, target.get())) {
			agent->setVel(SpaceVect::ray(agent->getMaxSpeed(), agent->getAngle()));
		}
		else {
			agent->setVel(SpaceVect::zero);
		}
	}

	return_pop_if_false(target.isValid());
}

bool FollowerMain::onEvent(Event event)
{
	bool handle = event.eventType == event_type::bulletHit;

	if (handle) {
		agent->rotate(float_pi * -0.5);
	}

	return handle;
}

void MarisaCollectMain::onEnter()
{
}

update_return MarisaCollectMain::update()
{
	GObject* player = getSpace()->getPlayer();

	if (player)
		return_push(ai::FollowPath::pathToTarget(fsm, player));
	else
		return_pop();
}

void MarisaForestMain::onEnter()
{
	gobject_ref player = fsm->getSpace()->getObjectRef("player");
	fsm->addThread(make_shared<ai::AimAtTarget>(fsm, player));
}

update_return MarisaForestMain::update()
{
	return_push(fsm->make<ai::Cast>(make_spell_generator<StarlightTyphoon>()));
}

ReimuYinYangOrbs::ReimuYinYangOrbs(StateMachine* fsm) :
	Function(fsm)
{
}

void ReimuYinYangOrbs::onEnter()
{
	Agent* agent = getAgent();
	for_irange(i, 0, orbCount)
	{
		SpaceFloat angle = float_pi * (0.25 + i * 0.5);
		auto params = Bullet::makeParams(
			agent->getPos() + SpaceVect::ray(1.5, angle),
			angle,
			SpaceVect::zero,
			float_pi
		);
		auto props = app::getBullet("yinYangOrb");
		orbs[i] = getSpace()->createObject<BulletImpl>(
			params,
			agent->getBulletAttributes(props),
			props
		);
	}
}

bool ReimuYinYangOrbs::onEvent(Event event)
{
	if (event.eventType != event_type::zeroHP) return false;

	for_irange(i, 0, orbCount)
	{
		getSpace()->removeObject(orbs[i].get());
	}

	return true;
}

const SpaceFloat RumiaMain1::dsdDistMargin = 5.0;
const SpaceFloat RumiaMain1::dsdLength = 5.0;
const SpaceFloat RumiaMain1::dsdCooldown = 15.0;
const float RumiaMain1::dsdCost = 5.0f;

RumiaMain1::RumiaMain1(ai::StateMachine* fsm, gobject_ref target) :
	ai::Function(fsm),
	target(target)
{
}

void RumiaMain1::onEnter()
{
	fsm->addThread(make_shared<ai::Flank>(fsm, target, 3.0, 1.0));
	fsm->addThread(make_shared<ai::FireAtTarget>(fsm, target));
}

void RumiaMain1::onReturn()
{
	fsm->addThread(make_shared<ai::FireAtTarget>(fsm, target));
}

update_return RumiaMain1::update()
{
	timerDecrement(dsdTimer);

	auto& as = *agent->getAttributeSystem();
	bool canCast = dsdTimer <= 0.0 && as[Attribute::mp] >= dsdCost;
	bool willCast =
		distanceToTarget(agent->getPos(), target.get()->getPos()) < dsdDistMargin &&
		as.getMagicRatio() > as.getHealthRatio()
	;

	if (canCast && willCast) {
		dsdTimer = dsdCooldown;
		fsm->removeThread("FireAtTarget");
		return_push(fsm->make<ai::Cast>(make_spell_generator<DarknessSignDemarcation>(), dsdLength));
	}
	else {
		return_steady();
	}
}

void RumiaMain1::onExit()
{
	fsm->removeThread("Flank");
	fsm->removeThread("FireAtTarget");
}

const vector<double_pair> RumiaDSD2::demarcationSizeIntervals = {
	make_pair(9.0, float_pi * 8.0 / 3.0),
	make_pair(12.0, float_pi * 2.0 / 3.0),
	make_pair(10.0, float_pi * 4.0 / 3.0),
};

void RumiaDSD2::onEnter()
{
	agent->cast(make_shared<DarknessSignDemarcation2>(
		agent,
		demarcationSizeIntervals.at(0).second
	));
}

update_return RumiaDSD2::update()
{
	timerIncrement(timer);

	if (timer >= demarcationSizeIntervals.at(intervalIdx).first) {
		++intervalIdx;
		intervalIdx %= demarcationSizeIntervals.size();
		timer = 0.0;

		agent->cast(make_shared<DarknessSignDemarcation2>(
			agent,
			demarcationSizeIntervals.at(intervalIdx).second
		));
	}

	return_steady();
}

void RumiaDSD2::onExit()
{
	agent->stopSpell();
}

void SakuyaMain::onEnter()
{
}

update_return SakuyaMain::update()
{
	return_push(fsm->make<Cast>(make_spell_generator<IllusionDial>()));
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

void SakuyaNPC1::onEnter()
{

}

update_return SakuyaNPC1::update()
{
	return_push(fsm->make<Wander>(0.25, 0.75, 4.0, 1.0));
}

void StalkerMain::onEnter()
{
}

update_return StalkerMain::update()
{
	if (agent->getAttribute(Attribute::stamina) <= 0.0f) {
		agent->getAttributeSystem()->setFullStamina();
		return_push(fsm->make<Cast>(make_spell_generator<Teleport>()));
	}
	else {
		return_steady();
	}
}

}//end NS
