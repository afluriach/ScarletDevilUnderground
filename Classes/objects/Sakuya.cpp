//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "Sakuya.hpp"
#include "SakuyaSpell.hpp"

const string Sakuya::baseAttributes = "sakuya";

Sakuya::Sakuya(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Sakuya::initStateMachine() {
	addThread(make_shared<SakuyaMain>(&fsm));
}

void SakuyaMain::onEnter()
{
}

ai::update_return SakuyaMain::update()
{
	return_push( fsm->make<ai::Cast>(make_spell_generator<IllusionDial>()) );
}

const SpaceFloat IllusionDash::scale = 2.5;
const SpaceFloat IllusionDash::opacity = 0.25;
const SpaceFloat IllusionDash::speed = 10.0;

IllusionDash::IllusionDash(ai::StateMachine* fsm, SpaceVect _target) :
	ai::Function(fsm),
	target(_target)
{}

IllusionDash::IllusionDash(ai::StateMachine* fsm, const ValueMap& args) :
	ai::Function(fsm)
{
	auto t = args.at("target").asValueMap();

	target = SpaceVect(t.at("x").asFloat(), t.at("y").asFloat());
}

void IllusionDash::onEnter()
{
	SpaceVect disp = ai::displacementToTarget(agent, target);

	agent->setVel(disp.normalizeSafe()*speed);
	agent->space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		agent->spriteID,
		motionBlurStretch(disp.length() / speed, disp.toAngle(), opacity, scale)
	);
}

ai::update_return IllusionDash::update()
{
	SpaceVect disp = ai::displacementToTarget(agent, target);
	agent->setVel(disp.normalizeSafe()*speed);

	if (disp.lengthSq() < 0.125f) {
		agent->setVel(SpaceVect::zero);
		return_pop();
	}
	else {
		return_steady();
	}
}
