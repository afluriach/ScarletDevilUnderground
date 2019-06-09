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

	unordered_set<Agent*> targets = physicsContext->radiusQueryByType<Agent>(
		source,
		source->getPos(),
		radius,
		enum_bitwise_or3(GType, enemy, player, npc),
		PhysicsLayers::all
	);

	for (Agent* target : targets)
	{
		float scale = getExplosionScale(source, target, radius);
		SpaceFloat knockback = baseDamage.mag * 5.0f * scale;

		target->hit(baseDamage * scale);
		log("Hit %s at scale %f.", target->getName().c_str(), scale);
		applyKnockback(source, target, knockback);
	}

	unordered_set<BreakableWall*> walls = physicsContext->radiusQueryByType<BreakableWall>(
		source,
		source->getPos(),
		radius,
		GType::wall,
		PhysicsLayers::all
	);

	for (BreakableWall* bw : walls){
		bw->hit();
	}

	unordered_set<Bomb*> bombs = physicsContext->radiusQueryByType<Bomb>(
		source,
		source->getPos(),
		radius,
		GType::bomb,
		PhysicsLayers::all
	);

	for (Bomb* bomb : bombs) {
		bomb->detonate();
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

void applyKnockback(const GObject* source, GObject* target, SpaceFloat mag)
{
	SpaceVect d = ai::directionToTarget(source, target->getPos());
	target->applyImpulse(d * mag);
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
