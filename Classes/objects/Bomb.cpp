//
//  Bomb.cpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "Bomb.hpp"
#include "GSpace.hpp"
#include "Wall.hpp"

Bomb::Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	RegisterInit<Bomb>(this)
{
	if (!vel.isZero()) {
		setInitialVelocity(vel);
	}
}

void Bomb::init()
{
	countdown = getFuseTime();
}

void Bomb::update()
{
	GObject::update();

	timerDecrement(countdown);

	if (countdown <= 0.0) {
		detonate();
	}
}

void Bomb::detonate()
{
	unordered_set<Agent*> targets = space->radiusQueryByType<Agent>(
		this,
		getPos(),
		getBlastRadius(),
		enum_bitwise_or3(GType, enemy, player, wall),
		PhysicsLayers::all
	);

	AttributeMap baseEffect = getAttributeEffect();

	for (Agent* target : targets)
	{
		float scale = getScale(target);
		SpaceFloat knockback = baseEffect.at(Attribute::hp) * -5.0f * scale;

		if (target->getType() == GType::player) {
			target->hit({ {Attribute::hp, -1.0f} }, getMagicEffect(target));
			applyKnockback(target, knockback);
		}
		else if (target->getType() == GType::enemy) {
			target->hit(AttributeSystem::scale(baseEffect, scale), getMagicEffect(target));
			log("Hit %s at magnitude %f.", target->getName().c_str(), scale);
			applyKnockback(target, knockback);
		}
	}

	unordered_set<BreakableWall*> walls = space->radiusQueryByType<BreakableWall>(
		this,
		getPos(),
		getBlastRadius(),
		GType::wall,
		PhysicsLayers::all
	);

	for (BreakableWall* bw : walls)
	{
		bw->hit();
	}

	space->removeObjectWithAnimation(this, bombAnimationAction(getBlastRadius() / getRadius()));
}

float Bomb::getScale(const GObject* target)
{
	SpaceFloat dist = ai::distanceToTarget(this, target->getPos());
	SpaceFloat halfRadius = getBlastRadius() / 2.0;

	if (dist < halfRadius)
		return 1.0f;
	else if (dist >= halfRadius * 2.0)
		return 0.0f;
	else
		return 1.0f - (dist - halfRadius) / halfRadius;
}

void Bomb::applyKnockback(GObject* target, SpaceFloat mag)
{
	SpaceVect d = ai::directionToTarget(this, target->getPos());
	target->applyImpulse(d * mag);
}

PlayerBomb::PlayerBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	Bomb(space, id, pos, vel)
{
}

RedFairyBomb::RedFairyBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos, const SpaceVect& vel) :
	GObject(space, id, "", pos, 0.0),
	Bomb(space, id, pos, vel)
{
}
