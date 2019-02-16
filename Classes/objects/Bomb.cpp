//
//  Bomb.cpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Bomb.hpp"
#include "GSpace.hpp"
#include "Wall.hpp"

Bomb::Bomb(GSpace* space, ObjectIDType id, const SpaceVect& pos) :
	GObject(space, id, "", pos, 0.0),
	RegisterInit<Bomb>(this),
	RegisterUpdate<Bomb>(this)
{
}

void Bomb::init()
{
	countdown = getFuseTime();
}

void Bomb::update()
{
	timerDecrement(countdown);

	if (countdown <= 0.0) {
		detonate();
	}
}

void Bomb::detonate()
{
	set<Agent*> targets = space->radiusQueryByType<Agent>(
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
		SpaceFloat knockback = baseEffect.at(Attribute::hp) * -2.0f * scale;

		if (target->getType() == GType::player) {
			target->hit({ {Attribute::hp, -1.0f} }, getMagicEffect(target));
			applyKnockback(target, knockback);
		}
		else if (target->getType() == GType::wall) {
			BreakableWall* bw = dynamic_cast<BreakableWall*>(target);
			if (bw) bw->hit();
		}
		else if (target->getType() == GType::enemy) {
			target->hit(AttributeSystem::scale(baseEffect, scale), getMagicEffect(target));
			log("Hit %s at magnitude %f.", target->getName().c_str(), scale);
			applyKnockback(target, knockback);
		}
	}

	space->removeObject(this);
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

PlayerBomb::PlayerBomb(GSpace* space, ObjectIDType id, const SpaceVect& pos) :
	GObject(space, id, "", pos, 0.0),
	Bomb(space, id, pos)
{
}
