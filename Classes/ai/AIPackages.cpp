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
#include "GSpace.hpp"
#include "MiscMagicEffects.hpp"
#include "Patchouli.hpp"
#include "Player.hpp"
#include "Reimu.hpp"
#include "value_map.hpp"

namespace ai {

void maintain_distance(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<MaintainDistance>(4.5f, 1.5f);
	fsm->addWhileDetectHandler(GType::player, engage);
}

void circle_and_fire(StateMachine* fsm, const ValueMap& args)
{
	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<LookAround>(&sm, float_pi / 4.0));
		sm.addThread(make_shared<FireIfTargetVisible>(&sm, p));
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

	fsm->setAlertFunction([waypoint, angularPos, waypointValid](StateMachine& sm, Player* p)->void {
		if (waypointValid) {
			sm.addThread(make_shared<CircleAround>(&sm, waypoint, angularPos, float_pi / 4.0));
		}
		sm.addThread(make_shared<FireIfTargetVisible>(&sm, p));
	});
}

void flock(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<Flock>(fsm));
}

void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args)
{
	const Path* path = fsm->getSpace()->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		fsm->setAlertFunction([path](StateMachine& sm, Player* p) -> void {
			auto comp = make_shared<CompositeFunction>(&sm);
			comp->addFunction<FollowPath>(*path, true, true);
			comp->addFunction<LookTowardsFire>(true);
			comp->addFunction<FireOnStress>(5.0f);
			comp->addFunction<BlueFairyPowerAttack>();
			sm.addThread(comp);
		});
	}
}

//Used for Slime; should not try to use pathfinding
void engage_player_in_room(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<Seek>(false);
	fsm->addWhileDetectHandler(GType::player, engage);
}

void seek_player(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<Seek>(true);
	fsm->addWhileDetectHandler(GType::player, engage);
}

void flee_player(StateMachine* fsm, const ValueMap& args) {
	auto engage = makeTargetFunctionGenerator<Flee>(1.5);

	fsm->addThread(make_shared<IdleWait>(fsm));
	fsm->addWhileDetectHandler(GType::player, engage);
}

void idle(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<IdleWait>(fsm));
}

void wander(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<Wander>(fsm));
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
		}
	);

	fsm->addEndDetectFunction(
		GType::player,
		[=](StateMachine& sm, GObject* target) -> void {
			fsm->removeThread("Flee");
		}
	);

	fsm->addThread(wanderThread);
}

void bat(StateMachine* fsm, const ValueMap& args) {
	auto engage = makeTargetFunctionGenerator<BatMain>();
	fsm->addWhileDetectHandler(GType::player, engage);
}

void facer(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<FacerMain>(fsm));
}

void follower(StateMachine* fsm, const ValueMap& args) {
	fsm->addThread(make_shared<FollowerMain>(fsm));
}


void ghost_fairy(StateMachine* fsm, const ValueMap& args)
{
	fsm->getObject()->addMagicEffect(make_shared<GhostProtection>(fsm->getAgent()));

	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Flank>(&sm, target, 4.0, 0.75));
			sm.addThread(make_shared<FireAtTarget>(&sm, target));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
			sm.removeThread("FireAtTarget");
		}
	);
}

void red_fairy(StateMachine* fsm, const ValueMap& args)
{
	RedFairy* agent = dynamic_cast<RedFairy*>(fsm->getObject());
	auto bombgen = agent->getBombs();
	AITargetFunctionGenerator engage = [bombgen](StateMachine* fsm, GObject* target) -> shared_ptr<Function> {
		auto comp = make_shared<CompositeFunction>(fsm);
		comp->addFunction<ThrowBombs>(
			target,
			bombgen,
			4.0,
			4.0,
			3.0,
			1.5,
			RedFairy::bombCost
		);
		comp->addFunction<FireAtTarget>(target);
		comp->addFunction<MaintainDistance>(target, 3.0f, 0.5f);

		return comp;
	};

	agent->addMagicEffect(make_shared<RedFairyStress>(agent));

	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 1.5, 2.5, 2.0, 3.0));
	});

	fsm->addFleeBomb();
	fsm->addWhileDetectHandler(GType::player, engage);
	fsm->addFunction<ExplodeOnZeroHP>(bomb_damage(20.0f), 4.0);
}

void greenFairyEngage(StateMachine& sm, Player* p)
{
	sm.addThread(make_shared<Wander>(&sm, 0.75, 1.5, 2.0, 4.0));
	sm.addThread(make_shared<EvadePlayerProjectiles>(&sm));
	sm.addThread(make_shared<FireOnStress>(&sm, 5.0f));
}

void green_fairy1(StateMachine* fsm, const ValueMap& args)
{
	fsm->addFleeBomb();

	fsm->getObject()->addMagicEffect(make_shared<BulletSpeedFromHP>(
		fsm->getAgent(),
		make_pair(0.25f, 0.75f),
		make_pair(0.75f, 1.0f),
		0.5f,
		0.25f
	));

	fsm->setAlertFunction(&greenFairyEngage);
}

void green_fairy2(StateMachine* fsm, const ValueMap& args)
{
	fsm->addFleeBomb();

	fsm->getObject()->addMagicEffect(make_shared<BulletSpeedFromHP>(
		fsm->getAgent(),
		make_pair(0.25f, 0.5f),
		make_pair(0.5f, 1.0f),
		1.0f / 3.0f,
		0.25f
	));

	fsm->setAlertFunction(&greenFairyEngage);
}

void zombie_fairy(StateMachine* fsm, const ValueMap& args)
{
	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 2.0, 3.0, 1.5, 3.0));
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
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
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

void ghost_fairy_npc(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<Wander>(fsm));

	auto seek = makeTargetFunctionGenerator<Seek>(true, 1.5);
	fsm->addWhileDetectHandler(GType::player, seek);
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
		}
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

void sakuya_npc(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<SakuyaNPC1>(fsm));
}


void scorpion1(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<Scurry>(3.0, -1.0);
	fsm->addWhileDetectHandler(GType::player, engage);
}

void scorpion2(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<Flank>(1.0, 1.0);
	fsm->addWhileDetectHandler(GType::player, engage);
}

void stalker(StateMachine* fsm, const ValueMap& args)
{
	auto engage = makeTargetFunctionGenerator<Seek>(true);

	fsm->addThread<StalkerMain>();
	fsm->addWhileDetectHandler(GType::player, engage);

	fsm->getAgent()->getAttributeSystem()->setFullStamina();
	fsm->getAgent()->addMagicEffect(make_shared<DrainStaminaFromMovement>(fsm->getAgent()));
}

void evade_player_projectiles(StateMachine* fsm, const ValueMap& args) {
	fsm->addThread(make_shared<EvadePlayerProjectiles>(fsm));
}

#define package(name) {#name, &name}

const unordered_map<string, StateMachine::PackageType> StateMachine::packages = {
	package(maintain_distance),
	package(circle_and_fire),
	package(circle_around_point),
	package(flock),
	package(blue_fairy_follow_path),
	package(engage_player_in_room),
	package(seek_player),
	package(flee_player),
	package(idle),
	package(wander),
	package(wander_and_flee_player),
	package(bat),
	package(facer),
	package(follower),
	package(ghost_fairy),
	package(red_fairy),
	package(green_fairy1),
	package(green_fairy2),
	package(zombie_fairy),
	package(fairy2),
	package(ice_fairy),
	package(ghost_fairy_npc),
	package(collect_marisa),
	package(forest_marisa),
	package(patchouli_enemy),
	package(reimu_enemy),
	package(rumia1),
	package(rumia2),
	package(sakuya),
	package(sakuya_npc),
	package(scorpion1),
	package(scorpion2),
	package(stalker),
	package(evade_player_projectiles),
};

}//end NS

