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

const bool Bullet::logRicochets = true;

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
	SpaceVect v = getVel();
	SpaceVect _n = n.roundToNearestDirection(8);

	if (SpaceVect::dot(_n, v) <= 0.0) return ricochetCount != 0;

	if (ricochetCount == 0) return false;
	if (ricochetCount > 0) --ricochetCount;

	SpaceVect vv = ricochetVelocity(v, _n, 1.0);

	setVel(vv);

	if (logRicochets) {
		log(
			"%s, %d velocity from %f,%f to %f,%f",
			getName().c_str(),
			space->getFrame(),
			v.length(),
			v.toAngle(),
			vv.length(),
			vv.toAngle()
		);
	}

	return true;
}

void Bullet::setBodyVisible(bool b)
{
	if (drawNodeID != 0) {
		space->setSpriteVisible(drawNodeID, b);
	}
}
