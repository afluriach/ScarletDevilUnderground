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
#include "EnemyFunctions.hpp"
#include "EnemySpell.hpp"
#include "Fairy.hpp"
#include "FirePatternImpl.hpp"
#include "Graphics.h"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

const string RedFairy::baseAttributes = "redFairy";
const string RedFairy::properName = "Red Fairy";

const DamageInfo RedFairy::explosionEffect = bomb_damage(20.0f);
const SpaceFloat RedFairy::explosionRadius = 4.0;
const float RedFairy::bombCost = 20.0f;

RedFairy::RedFairy(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		25.0,
		collectible_id::hm2
	)
{
	setFirePattern("RedFairy");
}

ParamsGeneratorType RedFairy::getBombs()
{
	return [](shared_ptr<object_params> params) -> ObjectGeneratorType {
		return GObject::params_object_factory<RedFairyBomb>(params);
	};
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

	LightID light = space->addLightSource(
		CircleLightArea::create( explosionRadius, Color4F::RED, 0.25 ),
		getPos(),
		0.0
	);
	space->addGraphicsAction(&graphics_context::autoremoveLightSource, light, 1.0f);
}

const string Fairy2::baseAttributes = "fairy2";
const float Fairy2::lowHealthRatio = 0.5f;

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		40.0,
		collectible_id::hm2
	)
{}

void Fairy2::addFleeThread()
{
	GObject* player = space->getPlayer();
	auto fleeThread = make_shared<ai::Thread>(
		make_shared<ai::Flee>(fsm.get(), player, 5.0f),
		fsm.get()
	);
	fsm->addThread(fleeThread);
}

void Fairy2::addSupportThread(object_ref<Fairy2> other)
{
	gobject_ref player = space->getObjectRef("player");

	if (!player.isValid()) {
		return;
	}

	supportThread = make_shared<ai::Thread>(
		make_shared<ai::OccupyMidpoint>(fsm.get(), other.getBaseRef(), player),
		fsm.get()
	);
	fsm->addThread(supportThread);
}

void Fairy2::removeSupportThread()
{
	if (supportThread) {
		fsm->removeThread(supportThread);
		supportThread.reset();
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
