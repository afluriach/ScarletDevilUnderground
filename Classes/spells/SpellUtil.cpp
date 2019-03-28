//
//  SpellUtil.cpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "App.h"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "SpellUtil.hpp"
#include "Wall.hpp"

void explosion(const GObject* source, SpaceFloat radius, AttributeMap baseEffect)
{
	unordered_set<Agent*> targets = source->space->radiusQueryByType<Agent>(
		source,
		source->getPos(),
		radius,
		enum_bitwise_or3(GType, enemy, player, wall),
		PhysicsLayers::all
	);

	for (Agent* target : targets)
	{
		float scale = getExplosionScale(source, target, radius);
		float sensitivity = target->getAttribute(Attribute::bombSensitivity);
		SpaceFloat knockback = baseEffect.at(Attribute::hp) * -5.0f * scale;

		target->hit(AttributeSystem::scale(baseEffect, scale * sensitivity), nullptr);
		log("Hit %s at magnitude %f.", target->getName().c_str(), scale);
		applyKnockback(source, target, knockback);
	}

	unordered_set<BreakableWall*> walls = source->space->radiusQueryByType<BreakableWall>(
		source,
		source->getPos(),
		radius,
		GType::wall,
		PhysicsLayers::all
	);

	for (BreakableWall* bw : walls){
		bw->hit();
	}

	App::playSoundSpatial("sfx/bomb_explosion.wav", toFmod(source->getPos()), toFmod(SpaceVect::zero));
}

float getExplosionScale(const GObject* source, const GObject* target, SpaceFloat radius)
{
	SpaceFloat dist = ai::distanceToTarget(source, target->getPos());
	SpaceFloat halfRadius = radius * 0.5;

	if (dist < halfRadius)
		return 1.0f;
	else if (dist >= radius)
		return 0.0f;
	else
		return 1.0f - (dist - halfRadius) / halfRadius;
}

void applyKnockback(const GObject* source, GObject* target, SpaceFloat mag)
{
	SpaceVect d = ai::directionToTarget(source, target->getPos());
	target->applyImpulse(d * mag);
}
