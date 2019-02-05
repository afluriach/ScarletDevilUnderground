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
		agentAttributes = agent->getAttributeSystem();
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

void Bullet::onAgentCollide(Agent* agent)
{
	agent->hit(getAttributeEffect(), getMagicEffect(agent));
	space->removeObject(this);
}

void Bullet::onBulletCollide(Bullet* bullet)
{
	//NO-OP
}

void Bullet::setBodyVisible(bool b)
{
	if (drawNodeID != 0) {
		space->setSpriteVisible(drawNodeID, b);
	}
}
