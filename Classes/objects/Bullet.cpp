//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Bullet.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"

bullet_attributes bullet_attributes::getDefault()
{
	return bullet_attributes{
		SpaceVect::zero,
		nullptr,
		GType::enemyBullet,
		1.0f,
		1.0f
	};
}


const bool Bullet::logRicochets = false;

Bullet::Bullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	const bullet_attributes& attributes,
	const physics_params& phys
) :
	GObject(PosAngleParams(pos,angle), phys),
	attributes(attributes)
{
}

void Bullet::onWallCollide(Wall* wall)
{
	if(!ignoreObstacleCollision)
		space->removeObject(this);
}

void Bullet::onEnvironmentCollide(GObject* obj)
{
	if(!ignoreObstacleCollision)
		space->removeObject(this);
}

void Bullet::onAgentCollide(Agent* other, SpaceVect n)
{
	if (hitCount > 0) --hitCount;

	if (getKnockbackForce() != 0.0) {
		other->applyImpulse(-n * getKnockbackForce());
	}

	if (hitCount == 0) {
		space->removeObject(this);
	}

	if (attributes.caster.isValid() ) {
		attributes.caster.get()->onBulletHitTarget(this, other);
	}
}

void Bullet::onBulletCollide(Bullet* bullet)
{
	if (deflectBullets && !bullet->deflectBullets && getType() != bullet->getType()) {
		space->removeObject(bullet);
	}
}

DamageInfo Bullet::getScaledDamageInfo() const
{
	auto damage = getDamageInfo();
	damage.mag *= attributes.attackDamage;
	return damage;
}

SpaceVect Bullet::calculateLaunchVelocity()
{
	SpaceFloat speed = getMaxSpeed() * attributes.bulletSpeed;
	SpaceFloat angle = getAngle();
	SpaceVect dir = SpaceVect::ray(1.0, angle);
	SpaceFloat scalar = SpaceVect::dot(dir, attributes.casterVelocity);
	scalar = scalar < 0.0 ? 0.0 : scalar;
	return dir * (speed + scalar);
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
		space->graphicsNodeAction(&Node::setVisible, drawNodeID, b);
	}
}

void Bullet::setShield(bool deflectBullets)
{
	hitCount = -1;
	ignoreObstacleCollision = true;
	this->deflectBullets = deflectBullets;
}

BulletImpl::BulletImpl(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	const bullet_attributes& attributes,
	shared_ptr<bullet_properties> props
) :
	Bullet(
		space,id,
		pos,angle,
		attributes,
		physics_params(props->radius, 0.1)
	),
	props(props)
{
	hitCount = props->hitCount;
	ricochetCount = props->ricochetCount;
}

void BulletImpl::init()
{
	GObject::init();

	if (props->directionalLaunch) {
		setVel(calculateLaunchVelocity());
	}
}

void BulletImpl::initializeGraphics()
{
	GObject::initializeGraphics();
}
