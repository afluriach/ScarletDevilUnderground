//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "Bullet.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"

Bullet::Bullet(Agent* agent)
{
	if (agent) {
		agentAttackMultiplier = agent->getAttributeSystem().getAttackMultiplier();
	}
}

void Bullet::onWallCollide(Wall* wall)
{
	space->removeObject(this);
}

void Bullet::onEnvironmentCollide(GObject* obj)
{
	space->removeObject(this);
}

void Bullet::onAgentCollide(Agent* agent, SpaceVect n)
{
	if (hitCount > 0) --hitCount;

	agent->hit(getAttributeEffect(), getMagicEffect(agent));

	if (knockback != 0.0) {
		agent->applyImpulse(-n * knockback);
	}

	if (hitCount == 0) {
		space->removeObject(this);
	}
}

void Bullet::onBulletCollide(Bullet* bullet)
{
	//NO-OP
}

bool Bullet::applyRicochet(SpaceVect n)
{
	if (ricochetCount == 0) return false;
	if (ricochetCount > 0) --ricochetCount;

	SpaceVect v = getVel();
	setVel(-2.0 * (SpaceVect::dot(v,n))*n + v);

	return true;
}

void Bullet::setBodyVisible(bool b)
{
	if (drawNodeID != 0) {
		space->setSpriteVisible(drawNodeID, b);
	}
}
