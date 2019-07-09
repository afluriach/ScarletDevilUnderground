//
//  EnemySpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "Bullet.hpp"
#include "Enemy.hpp"
#include "EnemySpell.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "physics_context.hpp"
#include "SpellUtil.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"

const string Teleport::name = "Teleport";
const string Teleport::description = "";
const float Teleport::cost = 0.0f;

Teleport::Teleport(GObject* caster) :
	Spell(caster),
	targets(getSpace()->getObjectsByTypeAs<TeleportPad>())
{
}

void Teleport::init()
{
}

void Teleport::update()
{
	bool success = false;

	for (auto ref : targets)
	{
		if (ref.isValid() && !ref.get()->isObstructed()) {
			log("%s teleported to %s.", caster->getName().c_str(), ref.get()->getName().c_str());
			caster->teleport(ref.get()->getPos());
			ref.get()->setTeleportActive(true);
			success = true;
			toUse = ref;
			break;
		}
	}

	if (success) {
		active = false;
	}
}

void Teleport::end()
{
	if (toUse.isValid()) {
		toUse.get()->setTeleportActive(false);
	}
}

const SpaceFloat TorchDarkness::radius = 2.5f;
const float TorchDarkness::effectMagnitude = 0.2f;

const string TorchDarkness::name = "TorchDarkness";
const string TorchDarkness::description = "";

TorchDarkness::TorchDarkness(GObject* caster) :
	Spell(caster)
{}

void TorchDarkness::update()
{
	unordered_set<Torch*> crntTorches = getSpace()->physicsContext->radiusQueryByType<Torch>(
		caster,
		caster->getPos(),
		radius,
		GType::environment,
		PhysicsLayers::all
	);

	for (Torch* crnt : crntTorches)
	{
		if (crnt->getActive())
		{
			crnt->applyDarkness(effectMagnitude * app::params.secondsPerFrame);
		}
	}
}

const DamageInfo BlueFairyBomb::damage = DamageInfo{
	10.0f,
	Attribute::end,
	DamageType::effectArea
};
const SpaceFloat BlueFairyBomb::length = 1.5;
const SpaceFloat BlueFairyBomb::radius = 2.5;
const SpaceFloat BlueFairyBomb::angularSpeed = float_pi * 0.5;

BlueFairyBomb::BlueFairyBomb(GObject* caster) :
	Spell(caster)
{}

void BlueFairyBomb::init()
{
	sprite = getSpace()->createSprite(
		&graphics_context::createSprite,
		string("sprites/blue_explosion.png"),
		GraphicsLayer::agentOverlay,
		toCocos(caster->getPos())*app::pixelsPerTile,
		to_float(radius * 0.5f)
	);
}

void BlueFairyBomb::update()
{
	timerIncrement(timer);

	getSpace()->graphicsNodeAction(
		&Node::setRotation,
		sprite,
		toCocosAngle(canonicalAngle(angularSpeed * timer))
	);
	radialEffectArea(caster, radius, GType::player, damage);

	if (timer >= length)
		active = false;
}

void BlueFairyBomb::end()
{
	getSpace()->addGraphicsAction(&graphics_context::removeSprite, sprite);
}

const string GreenFairyPowerAttack::name = "GreenFairyPowerAttack";
const string GreenFairyPowerAttack::description = "";

int GreenFairyPowerAttack::spawn()
{
	int spawnCount = 0;
	SpaceFloat angleStep = float_pi * 2.0 / bulletsPerWave;

	for_irange(i, 0, bulletsPerWave){
		 spawnCount += to_int(getCasterAs<Agent>()->bulletImplCheckSpawn<BulletImpl>(
			Bullet::makeParams(caster->getPos(),angleStep * i),
			 app::getBullet("greenFairyBullet")
		).isFuture());
	}
	return spawnCount;
}

void GreenFairyPowerAttack::update()
{
	timerIncrement(timer);

	if (timer >= interval)
	{
		spawn();
		timer -= interval;
		++wavesFired;

		active = (wavesFired < waveCount);
	}
}
