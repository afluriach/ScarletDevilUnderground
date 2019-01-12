//
//  Sakuya.cpp
//  Koumachika
//
//  Created by Toni on 4/17/18.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "GSpace.hpp"
#include "Sakuya.hpp"
#include "scenes.h"

const AttributeMap Sakuya::baseAttributes = {
	{Attribute::maxHP, 30.0f },
	{Attribute::speed, 3.0f},
	{Attribute::acceleration, 4.5f}
};

Sakuya::Sakuya(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::nil)
{}

void Sakuya::initStateMachine(ai::StateMachine& sm) {
	addThread(make_shared<SakuyaMain>());
}

void SakuyaMain::onEnter(ai::StateMachine& sm)
{
}

void SakuyaMain::update(ai::StateMachine& sm)
{
	sm.push(make_shared<ai::Cast>(make_spell_generator<IllusionDial>()));
}

const SpaceFloat IllusionDash::scale = 2.5;
const SpaceFloat IllusionDash::opacity = 0.25;
const SpaceFloat IllusionDash::speed = 10.0;

IllusionDash::IllusionDash(SpaceVect _target) :
	target(_target)
{}

IllusionDash::IllusionDash(GSpace* space, const ValueMap& args)
{
	auto t = args.at("target").asValueMap();

	target = SpaceVect(t.at("x").asFloat(), t.at("y").asFloat());
}

void IllusionDash::onEnter(ai::StateMachine& sm)
{
	SpaceVect disp = ai::displacementToTarget(sm.agent, target);

	sm.agent->setVel(disp.normalizeSafe()*speed);
	sm.agent->sprite->runAction(motionBlurStretch(disp.length() / speed, disp.toAngle(), opacity, scale));
}

void IllusionDash::update(ai::StateMachine& sm)
{
	SpaceVect disp = ai::displacementToTarget(sm.agent, target);
	sm.agent->setVel(disp.normalizeSafe()*speed);

	if (disp.lengthSq() < 0.125f) {
		sm.agent->setVel(SpaceVect::zero);
		sm.pop();
	}
}
