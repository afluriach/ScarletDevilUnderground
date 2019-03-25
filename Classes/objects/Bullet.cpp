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

const bool Bullet::logRicochets = false;

Bullet::Bullet(object_ref<Agent> agent) :
	agent(agent)
{
	if (agent.isValid()) {
		AttributeSystem& as = *agent.get()->getAttributeSystem();
		attributes.attackDamage = as[Attribute::attack];
		attributes.bulletSpeed = as[Attribute::bulletSpeed];
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

void Bullet::onAgentCollide(Agent* other, SpaceVect n)
{
	if (hitCount > 0) --hitCount;

	if (knockback != 0.0) {
		other->applyImpulse(-n * knockback);
	}

	if (hitCount == 0) {
		space->removeObject(this);
	}

	if (agent.isValid() ) {
		agent.get()->onBulletHitTarget(this, other);
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

BulletImpl::BulletImpl(const bullet_properties* props) :
	RegisterInit<BulletImpl>(this),
	props(props)
{
	hitCount = props->hitCount;
	ricochetCount = props->ricochetCount;
}

void BulletImpl::init()
{
	if (props->directionalLaunch)
		setVel(SpaceVect::ray(getMaxSpeed() * attributes.bulletSpeed, getAngle()));
}

void BulletImpl::initializeGraphics()
{
	ImageSprite::initializeGraphics();
	if (spriteID != 0 && props->spriteColor != Color3B::BLACK && props->spriteColor != Color3B::WHITE) {
		space->setSpriteColor(spriteID, props->spriteColor);
	}
}

BulletValueImpl::BulletValueImpl(bullet_properties props) :
	RegisterInit<BulletValueImpl>(this),
	props(props)
{
	hitCount = props.hitCount;
	ricochetCount = props.ricochetCount;
}

void BulletValueImpl::init()
{
	if (props.directionalLaunch)
		setVel(SpaceVect::ray(getMaxSpeed() * attributes.bulletSpeed, getAngle()));
}

void BulletValueImpl::initializeGraphics()
{
	ImageSprite::initializeGraphics();
	if (spriteID != 0 && props.spriteColor != Color3B::BLACK && props.spriteColor != Color3B::WHITE) {
		space->setSpriteColor(spriteID, props.spriteColor);
	}
}
