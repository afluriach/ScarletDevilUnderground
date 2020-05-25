//
//  AIPackages.cpp
//  Koumachika
//
//  Created by Toni on 6/22/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIFunctions.hpp"
#include "AIPackages.hpp"
#include "AIUtil.hpp"
#include "Bomb.hpp"
#include "Player.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"
#include "value_map.hpp"

namespace ai {

void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args)
{
	string name = fsm->getObject()->getName();
	const Path* path = fsm->getSpace()->getPath(name);

	if (path) {
		fsm->addAlertFunction([path](StateMachine* sm, Player* p) -> void {
			auto comp = make_local_shared<CompositeFunction>(sm);
			comp->addFunction<FollowPath>(*path, true, true);
			comp->addFunction<LookTowardsFire>(true);
			comp->addFunction<FireOnStress>(5.0f);
			comp->addFunction<ScriptFunction>("BlueFairyBomb");
			sm->addThread(comp);
		});
	}
}

void red_fairy(StateMachine* fsm, const ValueMap& args)
{
	Agent* agent = fsm->getAgent();

	AITargetFunctionGenerator engage = [](StateMachine* fsm, GObject* target) -> local_shared_ptr<Function> {
		auto comp = make_local_shared<CompositeFunction>(fsm);
		comp->addFunction<ThrowBombs>(
			target,
			app::getBomb("RedFairyBomb"),
			4.0,
			4.0
		);
		comp->addFunction<FireAtTarget>(target);
		comp->addFunction<MaintainDistance>(target, 3.0f, 0.5f);

		return comp;
	};

	fsm->addAlertFunction([](StateMachine* sm, Player* p)->void {
		sm->addThread(make_local_shared<Wander>(sm, 1.5, 2.5, 2.0, 3.0));
	});

	fsm->addFleeBomb();
	fsm->addWhileDetectHandler(GType::player, engage);
	fsm->addFunction<ExplodeOnZeroHP>(DamageInfo(20.0f, DamageType::bomb, Attribute::none, 100.0f), 4.0);
}

#define package(name) {#name, &name}

const unordered_map<string, StateMachine::PackageType> StateMachine::packages = {
	package(blue_fairy_follow_path),
	package(red_fairy),
};

}//end NS

