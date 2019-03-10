//
//  Fairy.cpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "EnemyFirePattern.hpp"
#include "Fairy.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "Player.hpp"
#include "Spell.hpp"
#include "value_map.hpp"

const AttributeMap Fairy1::baseAttributes = {
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

AttributeMap low_hp = {
	{ Attribute::maxHP, 20.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

AttributeMap high_hp = {
	{ Attribute::maxHP, 40.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

AttributeMap high_agility = {
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, 6.0f },
	{ Attribute::acceleration, 36.0f }
};

const AIPackage<Fairy1>::AIPackageMap Fairy1::aiPackages = {
	{"maintain_distance", &Fairy1::maintain_distance},
	{"flock", &Fairy1::flock},
	{"circle_and_fire", &Fairy1::circle_and_fire},
	{"circle_around_point", &Fairy1::circle_around_point}
};

const AttributePackageMap Fairy1::attributePackages = {
	{"high_agility", high_agility},
	{"low_hp", low_hp},
	{"high_hp", high_hp }
};

Fairy1::Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	AttributesPackage<Fairy1>(this, args),
	AIPackage<Fairy1>(this, args, "maintain_distance"),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 1.5, float_pi / 6.0, 3);
}

void Fairy1::maintain_distance(ai::StateMachine& sm, const ValueMap& args) {
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::MaintainDistance>(target, 4.5f, 1.5f));
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
		}
	);
}

void Fairy1::circle_and_fire(ai::StateMachine& sm, const ValueMap& args) {
	sm.setAlertFunction([](ai::StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<ai::LookAround>(float_pi / 4.0));
		sm.addThread(make_shared<ai::FireIfTargetVisible>(p));
	});
}

void Fairy1::circle_around_point(ai::StateMachine& sm, const ValueMap& args) {
	string waypointName = getStringOrDefault(args, "waypoint", "");
	SpaceVect waypoint;
	SpaceFloat angularPos;
	bool waypointValid = false;

	if (!waypointName.empty()){
		waypoint = space->getWaypoint(waypointName);
		angularPos = ai::directionToTarget(this, waypoint).toAngle() + float_pi;
		waypointValid = true;
	}

	sm.setAlertFunction([waypoint,angularPos,waypointValid](ai::StateMachine& sm, Player* p)->void {
		if (waypointValid) {
			sm.addThread(make_shared<ai::CircleAround>(waypoint, angularPos, float_pi / 4.0));
		}
		sm.addThread(make_shared<ai::FireIfTargetVisible>(p));
	});
}

void Fairy1::flock(ai::StateMachine& sm, const ValueMap& args) {
	
	shared_ptr<ai::Flock> flock = make_shared<ai::Flock>();

	sm.addThread(flock);

	sm.addDetectFunction(
		GType::enemy,
		[this, flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->onDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);

	sm.addEndDetectFunction(
		GType::enemy,
		[this, flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->endDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);
}

const AttributeMap FairyMaid::baseAttributes = {
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

const AIPackage<FairyMaid>::AIPackageMap FairyMaid::aiPackages = {
	{ "flee_player", &FairyMaid::flee_player },
	{ "idle", &FairyMaid::idle },
	{ "wander", &FairyMaid::wander },
};

FairyMaid::FairyMaid(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	DialogImpl(args),
	AIPackage<FairyMaid>(this, args, "idle")
{
}

void FairyMaid::flee_player(ai::StateMachine& sm, const ValueMap& args) {
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Flee>(target, 1.5), 1);
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Flee");
		}
	);
	sm.addThread(make_shared<ai::IdleWait>(), 0);
}

void FairyMaid::idle(ai::StateMachine& sm, const ValueMap& args)
{
	sm.addThread(make_shared<ai::IdleWait>());
}

void FairyMaid::wander(ai::StateMachine& sm, const ValueMap& args)
{
	sm.addThread(make_shared<ai::Wander>());
}

bool BlueFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	int level = getIntOrDefault(args, "level", 0);
	return level > App::crntState->getBlueFairyLevel();
}

BlueFairyNPC::BlueFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{
	level = getIntOrDefault(args, "level", 0);
}

string BlueFairyNPC::getDialog()
{
	if (level > App::crntState->getBlueFairyLevel() + 1) return "dialogs/blue_fairy_no";
	else {
		if (App::crntState->mushroomCount >= level) return "dialogs/blue_fairy_satisfied";
		else return "dialogs/blue_fairy_request_"+boost::lexical_cast<string>(level);
	}
}

void BlueFairyNPC::onDialogEnd()
{
	if (level == App::crntState->getBlueFairyLevel() + 1 && App::crntState->mushroomCount >= level) {
		++App::crntState->blueFairies;
		App::crntState->mushroomCount -= level;
		space->removeObject(this);
	} 
}

bool GhostFairyNPC::conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args)
{
	return true;
}

GhostFairyNPC::GhostFairyNPC(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent)
{
}

void GhostFairyNPC::initStateMachine(ai::StateMachine& sm)
{
	sm.addThread(make_shared<ai::Wander>());
}


const AIPackage<BlueFairy>::AIPackageMap BlueFairy::aiPackages = {
	{ "follow_path", &BlueFairy::follow_path },
};

const AttributeMap BlueFairy::baseAttributes = {
	{ Attribute::shieldActive, 1.0f },
	{ Attribute::shieldLevel, 1.0f },
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

BlueFairy::BlueFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	AIPackage(this, args, ""),
	Enemy(collectible_id::magic1)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 1.5, float_pi / 6.0, 3);
}

void BlueFairy::follow_path(ai::StateMachine& sm, const ValueMap& args)
{
	const Path* path = space->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		sm.setAlertFunction([path](ai::StateMachine& sm, Player* p) -> void {
			sm.addThread(make_shared<ai::FollowPath>(*path, true, true), 1);
			sm.addThread(make_shared<ai::LookTowardsFire>(), 2);
		});
	}
}

const AttributeMap RedFairy::baseAttributes = {
	{ Attribute::maxHP, 50.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 6.0f }
};

RedFairy::RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::health1),
	RegisterUpdate<RedFairy>(this)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 3.0, float_pi / 6.0, 2);
}

void RedFairy::update()
{
	//set fire rate and damage based on stress
	attributeSystem.timerDecrement(Attribute::stress);
	attributeSystem.setAttribute(Attribute::attackSpeed, 1.0f + max(25.0f, getAttribute(Attribute::stress))/25.0f);
}

void RedFairy::initStateMachine(ai::StateMachine& sm)
{
	sm.setAlertFunction([](ai::StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<ai::Wander>(1.5, 2.5, 2.0, 3.0), 0);
	});

	sm.setBulletHitFunction(ai::buildStressFromHits(1.0f));

	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::FireAtTarget>(target), 1);
			sm.addThread(make_shared<ai::MaintainDistance>(target, 3.0f, 0.5f), 1);
		}
	);

	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("FireAtTarget");
			sm.removeThread("MaintainDistance");
		}
	);
}

const AttributeMap GreenFairy::baseAttributes = {
	{ Attribute::maxHP, 30.0f },
	{ Attribute::speed, 3.0f },
	{ Attribute::acceleration, 4.5f }
};

GreenFairy::GreenFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<GreenFairyBulletPattern>(this, 1.5, 8);
}

void GreenFairy::initStateMachine(ai::StateMachine& sm)
{
	sm.setAlertFunction([](ai::StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<ai::Wander>(0.75, 1.5, 2.0, 4.0), 0);
		sm.addThread(make_shared<ai::EvadePlayerProjectiles>(), 1);
		sm.addThread(make_shared<ai::FireOnStress>(5.0f));
		sm.addThread(make_shared<ai::BuildStressFromPlayerProjectiles>(0.25f));
	});

	sm.setBulletHitFunction(ai::buildStressFromHits(0.5f));
}

const AttributeMap ZombieFairy::baseAttributes = {
	{ Attribute::maxHP, 75.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 10.0f },
	{ Attribute::darknessSensitivity, 0.0f }
};

ZombieFairy::ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::health2),
	RegisterInit<ZombieFairy>(this)
{
}

void ZombieFairy::init()
{
	cast(make_shared<TorchDarkness>(this));
}

void ZombieFairy::initStateMachine(ai::StateMachine& sm)
{
	sm.setAlertFunction([](ai::StateMachine& sm, Player* p)->void {
		sm.addThread(make_shared<ai::Wander>(2.0, 3.0, 1.5, 3.0), 1);
	});

	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::Seek>(target, true), 2);
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("Seek");
		}
	);
}

const AttributeMap Fairy2::baseAttributes = {
	{Attribute::maxHP, 15.0f},
	{Attribute::speed, 4.5f},
	{Attribute::acceleration, 4.5f}
};

const boost::rational<int> Fairy2::lowHealthRatio = boost::rational<int>(1, 3);

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2),
	RegisterUpdate<Fairy2>(this)
{}


void Fairy2::initStateMachine(ai::StateMachine& sm) {
	sm.addDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f), to_int(ai_priority::engage));
		}
	);
	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("MaintainDistance");
		}
	);
}

void Fairy2::addFleeThread()
{
	GObject* player = space->getObject("player");
	auto fleeThread = make_shared<ai::Thread>(
		make_shared<ai::Flee>(player, 5.0f),
		&fsm,
		to_int(ai_priority::flee),
		bitset<ai::lockCount>()
	);
	fsm.addThread(fleeThread);
}

void Fairy2::addSupportThread(object_ref<Fairy2> other)
{
	gobject_ref player = space->getObjectRef("player");

	if (!player.isValid()) {
		return;
	}

	auto t = make_shared<ai::Thread>(
		make_shared<ai::OccupyMidpoint>(other.getBaseRef(), player),
		&fsm,
		to_int(ai_priority::support),
		bitset<ai::lockCount>()
	);
	supportThread = t->uuid;

	fsm.addThread(t);
}

void Fairy2::removeSupportThread()
{
	if (supportThread != 0) {
		fsm.removeThread(supportThread);
		supportThread = 0;
	}
}

void Fairy2::update()
{
	if (getHealth() / getMaxHealth() <= lowHealthRatio && (crntState == ai_state::normal || crntState == ai_state::supporting)){
		space->messageAll(this, &Fairy2::requestHandler, &Fairy2::responseHandler, object_ref<Fairy2>(this));

		crntState = ai_state::flee;
		addFleeThread();
	}
}

object_ref<Fairy2> Fairy2::requestHandler(object_ref<Fairy2> other)
{
	if (other.isValid() && crntState == ai_state::normal) {
		crntState = ai_state::supportOffered;
		return this;
	}
	else {
		return nullptr;
	}
}

void Fairy2::responseHandler(object_ref<Fairy2> supporting)
{
	if (supporting.isValid() && crntState == ai_state::flee)
	{
		crntState = ai_state::fleeWithSupport;
		supporting.get()->message(supporting.get(), &Fairy2::acknowledgeHandaler, object_ref<Fairy2>(this));
	}
}

void Fairy2::acknowledgeHandaler(object_ref<Fairy2> supportTarget)
{
	crntState = ai_state::supporting;
	addSupportThread(supportTarget);
}

const AttributeMap IceFairy::baseAttributes = {
	{Attribute::maxHP, 15.0f},
	{Attribute::speed, 4.5f},
	{Attribute::acceleration, 4.5f},
	{Attribute::iceSensitivity, 0.0f}
};

IceFairy::IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::magic2)
{
	firePattern = make_shared<IceFairyBulletPattern>(this);
}

void IceFairy::initStateMachine(ai::StateMachine& sm) {
	Agent *const _agent = sm.getAgent();

	sm.addDetectFunction(
		GType::player,
		[_agent](ai::StateMachine& sm, GObject* target) -> void {
			sm.addThread(make_shared<ai::FireAtTarget>(target));
			sm.addThread(make_shared<ai::MaintainDistance>(target, 3.0f, 1.0f));
		}
	);

	sm.addEndDetectFunction(
		GType::player,
		[](ai::StateMachine& sm, GObject* target) -> void {
			sm.removeThread("FireAtTarget");
			sm.removeThread("MaintainDistance");
		}
	);
}
