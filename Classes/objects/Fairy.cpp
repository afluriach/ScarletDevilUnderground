//
//  Fairy.cpp
//  Koumachika
//
//  Created by Toni on 11/30/18.
//
//

#include "Prefix.h"

#include "AIFunctions.hpp"
#include "AIUtil.hpp"
#include "app_constants.hpp"
#include "audio_context.hpp"
#include "Bomb.hpp"
#include "EnemyFirePattern.hpp"
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"
#include "Fairy.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

const AttributeMap GhostFairy::baseAttributes = {
	{ Attribute::maxHP, 75.0f },
	{ Attribute::agility, 2.5f },
	{ Attribute::bulletSensitivity, 1.25},
};

const string GhostFairy::properName = "Ghost Fairy";

GhostFairy::GhostFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 1.5, float_pi / 6.0, 3);
}

void GhostFairy::initStateMachine()
{
	addMagicEffect(make_shared<GhostProtection>(object_ref<Agent>(this)));

	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Flank>(&fsm, target, 4.0, 0.75));
			fsm.addThread(make_shared<ai::FireAtTarget>(&fsm, target));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("MaintainDistance");
			fsm.removeThread("FireAtTarget");
		}
	);
}

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

void Fairy1::maintain_distance(const ValueMap& args) {
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::MaintainDistance>(&fsm, target, 4.5f, 1.5f));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("MaintainDistance");
		}
	);
}

void Fairy1::circle_and_fire(const ValueMap& args) {
	fsm.setAlertFunction([this](ai::StateMachine& sm, Player* p)->void {
		fsm.addThread(make_shared<ai::LookAround>(&fsm, float_pi / 4.0));
		fsm.addThread(make_shared<ai::FireIfTargetVisible>(&fsm, p));
	});
}

void Fairy1::circle_around_point(const ValueMap& args) {
	string waypointName = getStringOrDefault(args, "waypoint", "");
	SpaceVect waypoint;
	SpaceFloat angularPos;
	bool waypointValid = false;

	if (!waypointName.empty()){
		waypoint = space->getWaypoint(waypointName);
		angularPos = ai::directionToTarget(this, waypoint).toAngle() + float_pi;
		waypointValid = true;
	}

	fsm.setAlertFunction([this, waypoint,angularPos,waypointValid](ai::StateMachine& sm, Player* p)->void {
		if (waypointValid) {
			fsm.addThread(make_shared<ai::CircleAround>(&fsm, waypoint, angularPos, float_pi / 4.0));
		}
		fsm.addThread(make_shared<ai::FireIfTargetVisible>(&fsm, p));
	});
}

void Fairy1::flock(const ValueMap& args) {
	
	shared_ptr<ai::Flock> flock = make_shared<ai::Flock>(&fsm);

	fsm.addThread(flock);

	fsm.addDetectFunction(
		GType::enemy,
		[this, flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->onDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);

	fsm.addEndDetectFunction(
		GType::enemy,
		[this, flock](ai::StateMachine& sm, GObject* target) -> void {
			flock->endDetectNeighbor(dynamic_cast<Agent*>(target));
		}
	);
}

const AIPackage<BlueFairy>::AIPackageMap BlueFairy::aiPackages = {
	{ "follow_path", &BlueFairy::follow_path },
};

const AttributeMap BlueFairy::baseAttributes = {
	{ Attribute::shieldLevel, 1.0f },
	{ Attribute::maxHP, 90.0f },
	{ Attribute::maxMP, 75.0f },
	{ Attribute::maxStamina, 50.0f },
	{ Attribute::staminaRegen, 0.02f },
	{ Attribute::agility, 2.5f },
	{ Attribute::touchDamage, 5.0f },
	{ Attribute::stressFromHits, 1.5f },
	{ Attribute::stressFromBlocks, 1.0f },
	{ Attribute::bombSensitivity, 1.25f },
	{ Attribute::bulletSensitivity, 0.75f },
	{ Attribute::meleeSensitivity, 0.75f },
};

const string BlueFairy::properName = "Blue Fairy";

BlueFairy::BlueFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	AIPackage(this, args, ""),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<BlueFairyFirePattern>(this);
}

void BlueFairy::follow_path(const ValueMap& args)
{
	const Path* path = space->getPath(getStringOrDefault(args, "pathName", ""));

	if (path) {
		fsm.setAlertFunction([this, path](ai::StateMachine& sm, Player* p) -> void {
			fsm.addThread(make_shared<ai::FollowPath>(&fsm, *path, true, true), 1);
			fsm.addThread(make_shared<ai::LookTowardsFire>(&fsm, true), 2);
			fsm.addThread(make_shared<ai::FireOnStress>(&fsm, 5.0f));
			fsm.addThread(make_shared<ai::BlueFairyPowerAttack>(&fsm));
		});
	}
}

const AttributeMap RedFairy::baseAttributes = {
	{ Attribute::maxHP, 120.0f },
	{ Attribute::maxMP, 80.0f },
	{ Attribute::agility, 1.5f },
	{ Attribute::touchDamage, 10.0f },
	{ Attribute::stressDecay, 1.0f },
	{ Attribute::stressFromHits, 1.0f },
	{ Attribute::bombSensitivity, 0.5f }, 
	{ Attribute::bulletSensitivity, 0.75f },
	{ Attribute::meleeSensitivity, 1.25f },
};

const string RedFairy::properName = "Red Fairy";

const DamageInfo RedFairy::explosionEffect = bomb_damage(20.0f);
const SpaceFloat RedFairy::explosionRadius = 4.0;
const float RedFairy::bombCost = 20.0f;

RedFairy::RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 3.0, float_pi / 6.0, 2);
}

BombGeneratorType RedFairy::getBombs()
{
	return [](const SpaceVect& pos, const SpaceVect& vel) -> ObjectGeneratorType {
		return [=](GSpace* space, ObjectIDType id) -> GObject* {
			return new RedFairyBomb(space, id, pos, vel);
		};
	};
}

void RedFairy::initStateMachine()
{
	auto bombgen = getBombs();
	addMagicEffect(make_shared<RedFairyStress>(object_ref<Agent>(this)));
	
	fsm.setAlertFunction([this](ai::StateMachine& sm, Player* p)->void {
		fsm.addThread(make_shared<ai::Wander>(&fsm, 1.5, 2.5, 2.0, 3.0), 0);
	});

	fsm.addDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (fsm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				fsm.addThread(make_shared<ai::Flee>(&fsm, target, bomb->getBlastRadius()), 2);
			}
		}
	);

	fsm.addEndDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Flee");
		}
	);

	fsm.addDetectFunction(
		GType::player,
		[this, bombgen](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::ThrowBombs>(
				&fsm,
				target,
				bombgen,
				4.0,
				4.0,
				3.0,
				1.5,
				bombCost
			));
			fsm.addThread(make_shared<ai::FireAtTarget>(&fsm, target), 1);
			fsm.addThread(make_shared<ai::MaintainDistance>(&fsm, target, 3.0f, 0.5f), 1);
		}
	);

	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("ThrowBombs");
			fsm.removeThread("FireAtTarget");
			fsm.removeThread("MaintainDistance");
		}
	);
}

void RedFairy::onZeroHP()
{
	Agent::onZeroHP();
	explosion(this, explosionRadius, explosionEffect);
	SpriteID bombSprite = space->createSprite(
		&graphics_context::createSprite,
		string("sprites/explosion.png"),
		GraphicsLayer::overhead,
		toCocos(getPos()) * app::pixelsPerTile,
		1.0f
	);
	space->graphicsNodeAction(
		&Node::setColor,
		bombSprite,
		Color3B::RED
	);
	space->addGraphicsAction(
		&graphics_context::runSpriteAction,
		bombSprite,
		bombAnimationAction(explosionRadius / Bomb::explosionSpriteRadius, true)
	);
	space->audioContext->playSoundSpatial(
		"sfx/red_fairy_explosion.wav",
		toVec3(getPos()),
		toVec3(SpaceVect::zero)
	);

	LightID light = space->addLightSource(CircleLightArea::create( getPos(), explosionRadius, Color4F::RED, 0.25 ));
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
}

const AttributeMap GreenFairy1::baseAttributes = {
	{ Attribute::maxHP, 60.0f },
	{ Attribute::touchDamage, 3.0f },
	{ Attribute::stressFromHits, 0.5f },
	{ Attribute::agility, 3.0f }
};

const string GreenFairy1::properName = "Green Fairy I";

GreenFairy1::GreenFairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<GreenFairyBulletPattern>(this, 1.5, 8);
}

void GreenFairy1::initStateMachine()
{
	addMagicEffect(make_shared<BulletSpeedFromHP>(
		object_ref<Agent>(this),
		make_pair(0.25f, 0.75f),
		make_pair(0.75f, 1.0f),
		0.5f,
		0.25f
	));

	fsm.setAlertFunction([this](ai::StateMachine& sm, Player* p)->void {
		fsm.addThread(make_shared<ai::Wander>(&fsm, 0.75, 1.5, 2.0, 4.0), 0);
		fsm.addThread(make_shared<ai::EvadePlayerProjectiles>(&fsm), 1);
		fsm.addThread(make_shared<ai::FireOnStress>(&fsm, 5.0f));
	});

	fsm.addDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (fsm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				fsm.addThread(make_shared<ai::Flee>(&fsm, target, bomb->getBlastRadius()), 2);
			}
		}
	);

	fsm.addEndDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Flee");
		}
	);

}

const AttributeMap GreenFairy2::baseAttributes = {
	{ Attribute::maxHP, 90.0f },
	{ Attribute::touchDamage, 6.0f },
	{ Attribute::stressFromDetects, 0.25f },
	{ Attribute::stressFromHits, 0.5f },
	{ Attribute::agility, 4.0f }
};

const string GreenFairy2::properName = "Green Fairy II";

GreenFairy2::GreenFairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
	firePattern = make_shared<GreenFairyBulletPattern>(this, 1.5, 16);
}

void GreenFairy2::initStateMachine()
{
	addMagicEffect(make_shared<BulletSpeedFromHP>(
		object_ref<Agent>(this),
		make_pair(0.25f, 0.5f),
		make_pair(0.5f, 1.0f),
		1.0f / 3.0f,
		0.25f
	));

	fsm.setAlertFunction([this](ai::StateMachine& sm, Player* p)->void {
		fsm.addThread(make_shared<ai::Wander>(&fsm, 0.75, 1.5, 2.0, 4.0), 0);
		fsm.addThread(make_shared<ai::EvadePlayerProjectiles>(&fsm), 1);
		fsm.addThread(make_shared<ai::FireOnStress>(&fsm, 5.0f));
	});

	fsm.addDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			if (fsm.isThreadRunning("Flee")) return;
			if (Bomb* bomb = dynamic_cast<Bomb*>(target)) {
				fsm.addThread(make_shared<ai::Flee>(&fsm, target, bomb->getBlastRadius()), 2);
			}
		}
	);

	fsm.addEndDetectFunction(
		GType::bomb,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Flee");
		}
	);

}

void GreenFairy2::onRemove()
{
	Enemy::onRemove();

	if (firePattern) {
		firePattern->fireIfPossible();
	}
}

const AttributeMap ZombieFairy::baseAttributes = {
	{ Attribute::maxHP, 120.0f },
	{ Attribute::speed, 2.0f },
	{ Attribute::acceleration, 10.0f },
	{ Attribute::darknessSensitivity, 0.0f }
};

const string ZombieFairy::properName = "Zombie Fairy";

ZombieFairy::ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2),
	RegisterInit<ZombieFairy>(this)
{
}

void ZombieFairy::init()
{
	cast(make_shared<TorchDarkness>(this));
}

void ZombieFairy::initStateMachine()
{
	fsm.setAlertFunction([this](ai::StateMachine& sm, Player* p)->void {
		fsm.addThread(make_shared<ai::Wander>(&fsm, 2.0, 3.0, 1.5, 3.0), 1);
	});

	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::Seek>(&fsm, target, true), 2);
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("Seek");
		}
	);
}

const AttributeMap Fairy2::baseAttributes = {
	{Attribute::maxHP, 30.0f},
	{Attribute::agility, 2.0f},
};

const float Fairy2::lowHealthRatio = 0.5f;

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{}


void Fairy2::initStateMachine() {
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::MaintainDistance>(&fsm, target, 3.0f, 1.0f), to_int(ai_priority::engage));
		}
	);
	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("MaintainDistance");
		}
	);
}

void Fairy2::addFleeThread()
{
	GObject* player = space->getObject("player");
	auto fleeThread = make_shared<ai::Thread>(
		make_shared<ai::Flee>(&fsm, player, 5.0f),
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
		make_shared<ai::OccupyMidpoint>(&fsm, other.getBaseRef(), player),
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
	Enemy::update();

	if (getHealthRatio() <= lowHealthRatio && (crntState == ai_state::normal || crntState == ai_state::supporting)){

		crntState = ai_state::flee;
		addFleeThread();

		auto refs = space->getObjectsByTypeAs<Fairy2>();

		for (auto ref : refs) {
			Fairy2* f = ref.get();

			if (f->requestHandler(this).isValid()) {
				crntState = ai_state::fleeWithSupport;
				break;
			}
		}
	}
}

object_ref<Fairy2> Fairy2::requestHandler(object_ref<Fairy2> other)
{
	if (other.isValid() && crntState == ai_state::normal) {
		crntState = ai_state::supportOffered;
		addSupportThread(other);
		return this;
	}
	else {
		return nullptr;
	}
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

void IceFairy::initStateMachine() {
	fsm.addDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.addThread(make_shared<ai::FireAtTarget>(&fsm, target));
			fsm.addThread(make_shared<ai::MaintainDistance>(&fsm, target, 3.0f, 1.0f));
		}
	);

	fsm.addEndDetectFunction(
		GType::player,
		[this](ai::StateMachine& sm, GObject* target) -> void {
			fsm.removeThread("FireAtTarget");
			fsm.removeThread("MaintainDistance");
		}
	);
}
