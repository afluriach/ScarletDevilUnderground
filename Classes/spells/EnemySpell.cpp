//
//  EnemySpell.cpp
//  Koumachika
//
//  Created by Toni on 3/10/19.
//
//

#include "Prefix.h"

#include "app_constants.hpp"
#include "Enemy.hpp"
#include "EnemySpell.hpp"
#include "graphics_context.hpp"
#include "physics_context.hpp"
#include "SpellDescriptor.hpp"
#include "SpellUtil.hpp"

const string BlueFairyBomb::name = "BlueFairyBomb";
const string BlueFairyBomb::description = "";

const DamageInfo BlueFairyBomb::damage = DamageInfo(
	10.0f,
	DamageType::effectArea
);
const SpaceFloat BlueFairyBomb::length = 1.5;
const SpaceFloat BlueFairyBomb::radius = 2.5;
const SpaceFloat BlueFairyBomb::angularSpeed = float_pi * 0.5;

BlueFairyBomb::BlueFairyBomb(GObject* caster, local_shared_ptr<SpellDesc> desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ BlueFairyBomb::length, 0.0 })
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
