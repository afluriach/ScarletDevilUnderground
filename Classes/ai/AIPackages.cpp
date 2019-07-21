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
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<MaintainDistance>(&sm, target, 4.5f, 1.5f));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
		}
	);
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
	shared_ptr<Flock> flock = make_shared<Flock>(fsm);

	fsm->addThread(flock);

	fsm->addDetectFunction(
		GType::enemy,
		[flock](StateMachine& sm, GObject* target) -> void {
			flock->onDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);

	fsm->addEndDetectFunction(
		GType::enemy,
		[flock](StateMachine& sm, GObject* target) -> void {
			flock->endDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);
}

void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args)
{
	const Path* path = fsm->getSpace()->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		fsm->setAlertFunction([path](StateMachine& sm, Player* p) -> void {
			sm.addThread(make_shared<FollowPath>(&sm, *path, true, true));
			sm.addThread(make_shared<LookTowardsFire>(&sm, true));
			sm.addThread(make_shared<FireOnStress>(&sm, 5.0f));
			sm.addThread(make_shared<BlueFairyPowerAttack>(&sm));
		});
	}
}

//Used for Slime; should not try to use pathfinding
void engage_player_in_room(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Seek>(&sm, target, false));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);
}

void seek_player(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Seek>(&sm, target, true));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);
}

void flee_player(StateMachine* fsm, const ValueMap& args) {
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Flee>(&sm, target, 1.5));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
	fsm->addThread(make_shared<IdleWait>(fsm));
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
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<BatMain>(&sm));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("BatMain");
		}
	);
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
	agent->addMagicEffect(make_shared<RedFairyStress>(agent));

	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 1.5, 2.5, 2.0, 3.0));
	});

	fsm->addDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			if (sm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				sm.addThread(make_shared<Flee>(&sm, target, bomb->getBlastRadius()));
			}
		}
	);

	fsm->addEndDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);

	fsm->addDetectFunction(
		GType::player,
		[bombgen](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ThrowBombs>(
				&sm,
				target,
				bombgen,
				4.0,
				4.0,
				3.0,
				1.5,
				RedFairy::bombCost
			));
			sm.addThread(make_shared<FireAtTarget>(&sm, target));
			sm.addThread(make_shared<MaintainDistance>(&sm, target, 3.0f, 0.5f));
		}
	);

	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("ThrowBombs");
			sm.removeThread("FireAtTarget");
			sm.removeThread("MaintainDistance");
		}
	);
}
void green_fairy1(StateMachine* fsm, const ValueMap& args)
{
	fsm->getObject()->addMagicEffect(make_shared<BulletSpeedFromHP>(
		fsm->getAgent(),
		make_pair(0.25f, 0.75f),
		make_pair(0.75f, 1.0f),
		0.5f,
		0.25f
	));

	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 0.75, 1.5, 2.0, 4.0));
		sm.addThread(make_shared<EvadePlayerProjectiles>(&sm));
		sm.addThread(make_shared<FireOnStress>(&sm, 5.0f));
	});

	fsm->addDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			if (sm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				sm.addThread(make_shared<Flee>(&sm, target, bomb->getBlastRadius()));
			}
		}
	);

	fsm->addEndDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
}

void green_fairy2(StateMachine* fsm, const ValueMap& args)
{
	fsm->getObject()->addMagicEffect(make_shared<BulletSpeedFromHP>(
		fsm->getAgent(),
		make_pair(0.25f, 0.5f),
		make_pair(0.5f, 1.0f),
		1.0f / 3.0f,
		0.25f
	));

	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 0.75, 1.5, 2.0, 4.0));
		sm.addThread(make_shared<EvadePlayerProjectiles>(&sm));
		sm.addThread(make_shared<FireOnStress>(&sm, 5.0f));
	});

	fsm->addDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			if (sm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				sm.addThread(make_shared<Flee>(&sm, target, bomb->getBlastRadius()));
			}
		}
	);

	fsm->addEndDetectFunction(
		GType::bomb,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
}

void zombie_fairy(StateMachine* fsm, const ValueMap& args)
{
	fsm->setAlertFunction([](StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<Wander>(&sm, 2.0, 3.0, 1.5, 3.0));
	});

	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Seek>(&sm, target, true));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);

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
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<FireAtTarget>(&sm, target));
			sm.addThread(make_shared<MaintainDistance>(&sm, target, 3.0f, 1.0f));
		}
	);

	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("FireAtTarget");
			sm.removeThread("MaintainDistance");
		}
	);
}

void ghost_fairy_npc(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<Wander>(fsm));

	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("Seek"))
				sm.addThread(make_shared<Seek>(&sm, target, true, 1.5));
		}
	);
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

	fsm->addDetectFunction(
		GType::player,
		[agent](StateMachine& sm, GObject* target) -> void {
			if (!sm.isThreadRunning("ReimuMain")) {
				sm.getSpace()->createDialog("dialogs/reimu_forest_pre_fight", false);
				agent->lockDoors();
				agent->spawnOrbs();
				sm.addThread(make_shared<FireAtTarget>(&sm, target));
				sm.addThread(make_shared<Flank>(&sm, target, 3.0, 2.0));
			}
		}
	);
}

void rumia1(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			if (sm.getThreadCount() > 0) return;
			sm.getSpace()->createDialog("dialogs/rumia1", false);
			sm.addThread(make_shared<RumiaMain1>(&sm, target));
		}
	);
}

void rumia2(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			if (sm.getThreadCount() > 0) return;
			sm.getSpace()->createDialog("dialogs/rumia3", false);
			sm.addThread(make_shared<RumiaDSD2>(&sm));
			sm.addThread(make_shared<FireAtTarget>(&sm, target));
			sm.addThread(make_shared<Flank>(&sm, target, 3.0, 1.0));
		}
	);
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
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Scurry>(
				&sm,
				target,
				3.0,
				-1.0
			));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Scurry");
		}
	);
}

void scorpion2(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Flank>(&sm, target, 1.0, 1.0));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flank");
		}
	);
}

void stalker(StateMachine* fsm, const ValueMap& args)
{
	auto t1 = make_shared<ai::Thread>(
		make_shared<StalkerMain>(fsm),
		fsm
	);
	fsm->addThread(t1);

	fsm->addDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<Seek>(&sm, target, true));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);

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

