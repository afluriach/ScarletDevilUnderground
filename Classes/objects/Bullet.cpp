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
#include "MagicEffect.hpp"
#include "SpellSystem.hpp"

object_params Bullet::makeLaunchParams(
	SpaceVect pos,
	SpaceFloat angle,
	SpaceFloat speed,
	SpaceFloat angularVel,
	SpaceVect dimensions
) {
	object_params result;

	result.pos = pos;
	result.angle = angle;
	result.vel = SpaceVect::ray(speed,angle);
	result.angularVel = angularVel;
	result.dimensions = dimensions;

	return result;
}

object_params Bullet::makeParams(
	SpaceVect pos,
	SpaceFloat angle,
	SpaceVect vel,
	SpaceFloat angularVel,
	SpaceVect dimensions
) {
	object_params result;

	result.pos = pos;
	result.angle = angle;
	result.vel = vel;
	result.angularVel = angularVel;
	result.dimensions = dimensions;

	return result;
}

Bullet::Bullet(
	GSpace* space,
	ObjectIDType id,
	const object_params& params,
	const bullet_attributes& attributes,
	local_shared_ptr<bullet_properties> props
) :
	GObject(
		space,
		id,
		params,
		physics_params(
			attributes.type,
			PhysicsLayers::ground,
			0.0,
			true
		),
		props
	),
	attributes(attributes),
	props(props)
{
	hitCount = props->hitCount;
	ricochetCount = props->ricochetCount;

	crntRoom = attributes.startRoom;

	if (attributes.parametric) {
		setParametricMove(attributes.parametric);
	}
}

Bullet::~Bullet()
{
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
	if(!props->ignoreObstacles)
		space->removeObject(this);
}

void Bullet::onEnvironmentCollide(GObject* obj)
{
	if(!props->ignoreObstacles)
		space->removeObject(this);
}

void Bullet::onAgentCollide(Agent* other, SpaceVect n)
{
	if (hitCount > 0) --hitCount;

	if (hitCount == 0) {
		space->removeObject(this);
	}

	if (attributes.caster.isValid() ) {
		attributes.caster.getAs<Agent>()->onBulletHitTarget(this, other);
	}
}

void Bullet::onBulletCollide(Bullet* bullet)
{
	if (props->deflectBullets && !bullet->props->deflectBullets && getType() != bullet->getType()) {
		space->removeObject(bullet);
	}
}

DamageInfo Bullet::getScaledDamageInfo() const
{
	auto damage = getDamageInfo();
	damage.mag *= attributes.attackDamage;
	return damage;
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
			toString(),
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

void Bullet::onRemove()
{
	GObject::onRemove();

	if (attributes.sourceSpell) {
		space->spellSystem->onRemove(attributes.sourceSpell, this);
	}
}
