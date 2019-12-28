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
#include "physics_context.hpp"
#include "SpellUtil.hpp"
#include "TeleportPad.hpp"
#include "Torch.hpp"

const SpaceFloat TorchDarkness::radius = 2.5f;
const float TorchDarkness::effectMagnitude = 0.2f;

const string TorchDarkness::name = "TorchDarkness";
const string TorchDarkness::description = "";

TorchDarkness::TorchDarkness(GObject* caster) :
	Spell(caster, spell_params{ -1.0, 0.0 })
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
			crnt->hit(DamageInfo(effectMagnitude, DamageType::effectArea, Attribute::darknessDamage, 0.0f), SpaceVect::zero);
		}
	}
}

const DamageInfo BlueFairyBomb::damage = DamageInfo(
	10.0f,
	DamageType::effectArea
);
const SpaceFloat BlueFairyBomb::length = 1.5;
const SpaceFloat BlueFairyBomb::radius = 2.5;
const SpaceFloat BlueFairyBomb::angularSpeed = float_pi * 0.5;

BlueFairyBomb::BlueFairyBomb(GObject* caster) :
	Spell(caster, spell_params{ BlueFairyBomb::length, 0.0 })
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
	getSpace()->graphicsNodeAction(
		&Node::setRotation,
		sprite,
		toCocosAngle(canonicalAngle(angularSpeed * t))
	);
	radialEffectArea(caster, radius, GType::player, damage);
}

void BlueFairyBomb::end()
{
	getSpace()->addGraphicsAction(&graphics_context::removeSprite, sprite);
}
