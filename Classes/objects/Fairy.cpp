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
#include "GState.hpp"
#include "MiscMagicEffects.hpp"
#include "Player.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

const string Fairy2::baseAttributes = "fairy2";
const float Fairy2::lowHealthRatio = 0.5f;

Fairy2::Fairy2(GSpace* space, ObjectIDType id, const ValueMap& args) :
	Enemy(
		space, id, args,
		baseAttributes,
		defaultSize,
		40.0,
		"health2,magic2",
		false
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
