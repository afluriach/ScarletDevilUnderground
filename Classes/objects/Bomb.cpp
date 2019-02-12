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
	set<GObject*> targets = space->radiusQuery(
		this,
		getPos(),
		getBlastRadius(),
		enum_bitwise_or3(GType, enemy, player, wall),
		PhysicsLayers::all
	);

	AttributeMap baseEffect = getAttributeEffect();

	for (GObject* obj : targets)
	{
		Agent* a = dynamic_cast<Agent*>(obj);
		float scale = getScale(obj);
		SpaceFloat knockback = baseEffect.at(Attribute::hp) * -2.0f * scale;

		if (obj->getType() == GType::player) {
			a->hit({ {Attribute::hp, -1.0f} }, getMagicEffect(obj));
			applyKnockback(obj, knockback);
		}
		else if (obj->getType() == GType::wall) {
			BreakableWall* bw = dynamic_cast<BreakableWall*>(obj);
			if (bw) {
				bw->hit();
			}
		}
		else if (obj->getType() == GType::enemy) {
			a->hit(AttributeSystem::scale(baseEffect, scale), getMagicEffect(obj));
			log("Hit %s at magnitude %f.", obj->getName().c_str(), scale);
			applyKnockback(obj, knockback);
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
