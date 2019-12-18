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
#include "Bomb.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "physics_context.hpp"
#include "SpellUtil.hpp"
#include "Wall.hpp"

void explosion(const GObject* source, SpaceFloat radius, DamageInfo baseDamage)
{
	physics_context* physicsContext = source->space->physicsContext.get();

	unordered_set<GObject*> targets = physicsContext->radiusQuery(
		source,
		source->getPos(),
		radius,
		GType::canDamage,
		PhysicsLayers::all
	);

	for (GObject* target : targets)
	{
		DamageInfo crntDamage = baseDamage;

		float scale = getExplosionScale(source, target, radius);
		SpaceFloat knockback = baseDamage.mag * 5.0f * scale;

		crntDamage = crntDamage*scale;
		crntDamage.knockback = calculateExplosionKnockback(source, target, knockback);

		target->hit(crntDamage);
		log("Hit %s at scale %f.", target->getName().c_str(), scale);
	}
}

float getExplosionScale(const GObject* source, const GObject* target, SpaceFloat radius)
{
	return getExplosionScale(source->getPos(), target, radius);
}

float getExplosionScale(const SpaceVect& pos, const GObject* target, SpaceFloat radius)
{
	SpaceFloat dist = ai::distanceToTarget(pos, target->getPos());
	SpaceFloat halfRadius = radius * 0.5;

	if (dist < halfRadius)
		return 1.0f;
	else if (dist >= radius)
		return 0.0f;
	else
		return 1.0f - (dist - halfRadius) / halfRadius;
}

SpaceVect calculateExplosionKnockback(const GObject* source, GObject* target, SpaceFloat mag)
{
	SpaceVect d = ai::directionToTarget(source, target->getPos());
	return d * mag;
}

void radialEffectArea(const GObject* source, SpaceFloat radius, GType targets, DamageInfo damage)
{
	unordered_set<Agent*> agents = source->space->physicsContext->radiusQueryByType<Agent>(
		source,
		source->getPos(),
		radius,
		targets,
		PhysicsLayers::all
	);

	for (Agent* agent : agents){
		agent->hit(damage);
	}
}
