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

const string GhostFairy::baseAttributes = "ghostFairy";
const string GhostFairy::properName = "Ghost Fairy";

GhostFairy::GhostFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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

const string Fairy1::baseAttributes = "fairy1";

Fairy1::Fairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 1.5, float_pi / 6.0, 3);
}

const string BlueFairy::baseAttributes = "blueFairy";
const string BlueFairy::properName = "Blue Fairy";

BlueFairy::BlueFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm1)
{
	firePattern = make_shared<BlueFairyFirePattern>(this);
}

const string RedFairy::baseAttributes = "redFairy";
const string RedFairy::properName = "Red Fairy";

const DamageInfo RedFairy::explosionEffect = bomb_damage(20.0f);
const SpaceFloat RedFairy::explosionRadius = 4.0;
const float RedFairy::bombCost = 20.0f;

RedFairy::RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
	firePattern = make_shared<Fairy1BulletPattern>(this, 3.0, float_pi / 6.0, 2);
}

ParamsGeneratorType RedFairy::getBombs()
{
	return [](shared_ptr<object_params> params) -> ObjectGeneratorType {
		return GObject::params_object_factory<RedFairyBomb>(params);
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
	playSoundSpatial("sfx/red_fairy_explosion.wav");

	LightID light = space->addLightSource(CircleLightArea::create( getPos(), explosionRadius, Color4F::RED, 0.25 ));
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
}

const string GreenFairy1::baseAttributes = "greenFairy1";
const string GreenFairy1::properName = "Green Fairy I";

GreenFairy1::GreenFairy1(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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

const string GreenFairy2::baseAttributes = "greenFairy2";
const string GreenFairy2::properName = "Green Fairy II";

GreenFairy2::GreenFairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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

const string ZombieFairy::baseAttributes = "zombieFairy";
const string ZombieFairy::properName = "Zombie Fairy";

ZombieFairy::ZombieFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
	MapObjForwarding(Agent),
	Enemy(collectible_id::hm2)
{
}

void ZombieFairy::init()
{
	Agent::init();

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

const string Fairy2::baseAttributes = "fairy2";
const float Fairy2::lowHealthRatio = 0.5f;

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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

const string IceFairy::baseAttributes = "iceFairy"; 

IceFairy::IceFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjParams(),
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
