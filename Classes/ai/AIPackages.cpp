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
#include "EnemyFunctions.hpp"
#include "GSpace.hpp"
#include "value_map.hpp"

namespace ai {

void maintain_distance(StateMachine* fsm, const ValueMap& args)
{
	fsm->addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::MaintainDistance>(&sm, target, 4.5f, 1.5f));
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
		}
	);
}

void circle_and_fire(StateMachine* fsm, const ValueMap& args)
{
	fsm->setAlertFunction([](ai::StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<ai::LookAround>(&sm, float_pi / 4.0));
		sm.addThread(make_shared<ai::FireIfTargetVisible>(&sm, p));
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
		angularPos = ai::directionToTarget(fsm->getObject(), waypoint).toAngle() + float_pi;
		waypointValid = true;
	}

	fsm->setAlertFunction([waypoint, angularPos, waypointValid](ai::StateMachine& sm, Player* p)->void {
		if (waypointValid) {
			sm.addThread(make_shared<ai::CircleAround>(&sm, waypoint, angularPos, float_pi / 4.0));
		}
		sm.addThread(make_shared<ai::FireIfTargetVisible>(&sm, p));
	});
}

void flock(StateMachine* fsm, const ValueMap& args)
{
	shared_ptr<ai::Flock> flock = make_shared<ai::Flock>(fsm);

	fsm->addThread(flock);

	fsm->addDetectFunction(
		GType::enemy,
		[flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->onDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);

	fsm->addEndDetectFunction(
		GType::enemy,
		[flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->endDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);
}

void blue_fairy_follow_path(StateMachine* fsm, const ValueMap& args)
{
	const Path* path = fsm->getSpace()->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		fsm->setAlertFunction([path](ai::StateMachine& sm, Player* p) -> void {
			sm.addThread(make_shared<ai::FollowPath>(&sm, *path, true, true), 1);
			sm.addThread(make_shared<ai::LookTowardsFire>(&sm, true), 2);
			sm.addThread(make_shared<ai::FireOnStress>(&sm, 5.0f));
			sm.addThread(make_shared<ai::BlueFairyPowerAttack>(&sm));
		});
	}
}

void flee_player(StateMachine* fsm, const ValueMap& args) {
	fsm->addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Flee>(&sm, target, 1.5), 1);
		}
	);
	fsm->addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
	fsm->addThread(make_shared<ai::IdleWait>(fsm), 0);
}

void idle(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<ai::IdleWait>(fsm));
}

void wander(StateMachine* fsm, const ValueMap& args)
{
	fsm->addThread(make_shared<ai::Wander>(fsm));
}

#define package(name) {#name, &name}

const unordered_map<string, StateMachine::PackageType> StateMachine::packages = {
	package(maintain_distance),
	package(circle_and_fire),
	package(circle_around_point),
	package(flock),
	package(blue_fairy_follow_path),
	package(flee_player),
	package(idle),
	package(wander),
};

}//end NS

