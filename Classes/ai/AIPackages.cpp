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
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"
#include "Fairy.hpp"
#include "MiscMagicEffects.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Reimu.hpp"
#include "value_map.hpp"

namespace ai {

void circle_and_fire(StateMachine* fsm, const ValueMap& args)
{
	fsm->addAlertFunction([](StateMachine* sm, Player* p)->void {
		sm->addThread(make_shared<LookAround>(sm, float_pi / 4.0));
		sm->addThread(make_shared<FireIfTargetVisible>(sm, p));
	});
}

void circle_around_point(StateMachine* fsm, const ValueMap& args)
{
	string waypointName = getStringOrDefault(args, "waypoint", "");
	SpaceVect waypoint;
	SpaceFloat angularPos;
	bool waypointValid = false;

	if (!waypointName.empty()) {
		waypoint = fsm->getSpace()->getWaypoint(waypointName);
		angularPos = directionToTarget(fsm->getObject(), waypoint).toAngle() + float_pi;
		waypointValid = true;
	}

	fsm->addAlertFunction([waypoint, angularPos, waypointValid](StateMachine* sm, Player* p)->void {
		if (waypointValid) {
			sm->addThread(make_shared<CircleAround>(sm, waypoint, angularPos, float_pi / 4.0));
		}
		sm->addThread(make_shared<FireIfTargetVisible>(sm, p));
	});
}

void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args)
{
	const Path* path = fsm->getSpace()->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		fsm->addAlertFunction([path](StateMachine* sm, Player* p) -> void {
			auto comp = make_shared<CompositeFunction>(sm);
			comp->addFunction<FollowPath>(*path, true, true);
			comp->addFunction<LookTowardsFire>(true);
			comp->addFunction<FireOnStress>(5.0f);
			comp->addFunction<BlueFairyPowerAttack>();
			sm->addThread(comp);
		});
	}
}

void wander_and_flee_player(StateMachine* fsm, const ValueMap& args)
{
	auto wanderThread = make_shared<Thread>(
		make_shared<Wander>(fsm, 1.0, 3.0, 2.0, 4.0),
		fsm
	);

	fsm->addDetectFunction(
		GType::player,
		[wanderThread](StateMachine& sm, GObject* target) -> void {
			wanderThread->popToRoot();
			sm.addThread(make_shared<Flee>(&sm, target, 3.0f));
		},
		[=](StateMachine& sm, GObject* target) -> void {
			fsm->removeThread("Flee");
		}
	);

	fsm->addThread(wanderThread);
}

void ghost_fairy(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Flank>(&sm, target, 4.0, 0.75));
			sm.addThread(make_shared<FireAtTarget>(&sm, target));
		},
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
			sm.removeThread("FireAtTarget");
		}
	);
}

void red_fairy(StateMachine* fsm, const ValueMap& args)
{
	Agent* agent = fsm->getAgent();

	AITargetFunctionGenerator engage = [](StateMachine* fsm, GObject* target) -> shared_ptr<Function> {
		auto comp = make_shared<CompositeFunction>(fsm);
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
		sm->addThread(make_shared<Wander>(sm, 1.5, 2.5, 2.0, 3.0));
	});

	fsm->addFleeBomb();
	fsm->addWhileDetectHandler(GType::player, engage);
	fsm->addFunction<ExplodeOnZeroHP>(DamageInfo(20.0f, DamageType::bomb, Attribute::end, 100.0f), 4.0);
}

void greenFairyEngage(StateMachine* sm, Player* p)
{
	sm->addThread(make_shared<Wander>(sm, 0.75, 1.5, 2.0, 4.0));
	sm->addThread(make_shared<Evade>(sm, GType::playerBullet));
	sm->addThread(make_shared<FireOnStress>(sm, 5.0f));
}

void green_fairy1(StateMachine* fsm, const ValueMap& args)
{
	fsm->addFleeBomb();
	fsm->addAlertFunction(&greenFairyEngage);
}

void green_fairy2(StateMachine* fsm, const ValueMap& args)
{
	fsm->addFleeBomb();
	fsm->addAlertFunction(&greenFairyEngage);
}

void zombie_fairy(StateMachine* fsm, const ValueMap& args)
{
	fsm->addAlertFunction([](StateMachine* sm, Player* p)->void {
		sm->addThread(make_shared<Wander>(sm, 2.0, 3.0, 1.5, 3.0));
	});

	auto engage = makeTargetFunctionGenerator<Seek>(true);
	fsm->addWhileDetectHandler(GType::player, engage);

	fsm->getObject()->cast(make_shared<TorchDarkness>(fsm->getAgent()));
}

void fairy2(StateMachine* fsm, const ValueMap& args) {
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<MaintainDistance>(&sm, target, 3.0f, 1.0f));
		},
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
		}
	);
}

void ice_fairy(StateMachine* fsm, const ValueMap& args) {
	auto engage = [](StateMachine* fsm, GObject* target) -> shared_ptr<Function> {
		auto comp = make_shared<CompositeFunction>(fsm);
		comp->addFunction<FireAtTarget>(target);
		comp->addFunction<MaintainDistance>(target, 3.0f, 1.0f);
		return comp;
	};
	fsm->addWhileDetectHandler(GType::player, engage);
}

void collect_marisa(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<MarisaCollectMain>(fsm));
}

void forest_marisa(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<MarisaForestMain>(fsm));
}

void patchouli_enemy(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<HPCastSequence>(fsm, PatchouliEnemy::spells, makeIntervalMap(PatchouliEnemy::intervals)));
}

void reimu_enemy(StateMachine* fsm, const ValueMap& args)
{
	auto agent = dynamic_cast<ReimuEnemy*>(fsm->getObject());
	auto boss = make_shared<BossFightHandler>(fsm, "dialogs/reimu_forest_pre_fight", "dialogs/reimu_forest_post_fight");
	fsm->addFunction(boss);
	fsm->addFunction<ReimuYinYangOrbs>();

	fsm->addDetectFunction(
		GType::player,
		[agent](StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("Flank")) {
				sm.addThread(make_shared<FireAtTarget>(&sm, target));
				sm.addThread(make_shared<Flank>(&sm, target, 3.0, 2.0));
			}
		},
		[agent](StateMachine& sm, GObject* target) -> void {}
	);
}

void rumia1(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<RumiaMain1>();
	auto boss = make_shared<BossFightHandler>(fsm, "dialogs/rumia1", "dialogs/rumia2");

	fsm->addFunction(boss);
	fsm->addOnDetectHandler(GType::player, engage);
}

void rumia2(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<RumiaMain2>();
	auto boss = make_shared<BossFightHandler>(fsm, "dialogs/rumia3", "dialogs/rumia4");

	fsm->addFunction(boss);
	fsm->addOnDetectHandler(GType::player, engage);
}

void sakuya(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<SakuyaMain>(fsm));
}

#define package(name) {#name, &name}

const unordered_map<string, StateMachine::PackageType> StateMachine::packages = {
	package(circle_and_fire),
	package(circle_around_point),
	package(blue_fairy_follow_path),
	package(wander_and_flee_player),
	package(ghost_fairy),
	package(red_fairy),
	package(green_fairy1),
	package(green_fairy2),
	package(zombie_fairy),
	package(fairy2),
	package(ice_fairy),
	package(collect_marisa),
	package(forest_marisa),
	package(patchouli_enemy),
	package(reimu_enemy),
	package(rumia1),
	package(rumia2),
	package(sakuya),
};

}//end NS

