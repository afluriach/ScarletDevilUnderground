//
//  Bullet.cpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "app_constants.hpp"
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
		nullptr,
		1.0f,
		1.0f
	};
}

bullet_properties bullet_properties::clone() {
	return *this;
}

const bool Bullet::logRicochets = false;

shared_ptr<object_params> Bullet::makeParams(
	SpaceVect pos,
	SpaceFloat angle,
	SpaceVect vel,
	SpaceFloat angularVel
) {
	auto result = make_shared<object_params>();

	result->pos = pos;
	result->angle = angle;
	result->vel = vel;
	result->angularVel = angularVel;

	return result;
}

Bullet::Bullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes,
	const physics_params& phys
) :
	GObject(params, phys),
	attributes(attributes)
{
	crntRoom = attributes.startRoom;
}

void Bullet::initializeGraphics()
{
	GObject::initializeGraphics();

	drawNodeID = space->createSprite(
		&graphics_context::createDrawNode,
		GraphicsLayer::agentOverlay,
		getInitialCenterPix(),
		1.0f
	);

	if (dimensions.y == 0.0) {
		space->graphicsNodeAction(
			static_cast<void(DrawNode::*)(const Vec2&, float, float, unsigned int, const Color4F&)>(&DrawNode::drawSolidCircle),
			drawNodeID,
			Vec2::ZERO,
			to_float(dimensions.x * app::pixelsPerTile),
			0.0f,
			to_uint(32),
			Color4F(.66f, .75f, .66f, .7f)
		);
	}
	else {
		Vec2 pixelExtents = toCocos(dimensions) * app::pixelsPerTile * 0.5f;
		swap(pixelExtents.x, pixelExtents.y);
		space->graphicsNodeAction(
			&DrawNode::drawSolidRect,
			drawNodeID,
			-pixelExtents,
			pixelExtents,
			Color4F(.66f, .75f, .66f, .7f)
		);
	}

	space->graphicsNodeAction(&Node::setVisible, drawNodeID, false);
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
		other->applyImpulse(-1.0 * n * getKnockbackForce());
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
	shared_ptr<object_params> params,
	const bullet_attributes& attributes,
	shared_ptr<bullet_properties> props
) :
	Bullet(
		params,
		attributes,
		physics_params(attributes.type, PhysicsLayers::ground, props->dimensions, 0.0)
	),
	props(props)
{
	hitCount = props->hitCount;
	ricochetCount = props->ricochetCount;
	ignoreObstacleCollision = props->ignoreObstacles;
	deflectBullets = props->deflectBullets;
}

void BulletImpl::init()
{
	GObject::init();

	if (props->directionalLaunch) {
		setVel(calculateLaunchVelocity());
	}
}
