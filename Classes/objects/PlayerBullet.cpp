//
//  PlayerBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "GSpace.hpp"
#include "Player.hpp"
#include "PlayerBullet.hpp"

StarbowBreakBullet::StarbowBreakBullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes,
	shared_ptr<bullet_properties> props
) :
	BulletImpl(params, attributes, props)
{}

shared_ptr<LightArea> StarbowBreakBullet::getLightSource() const {
	return CircleLightArea::create(
		getPos(),
		props->dimensions.x * 4.0f,
		toColor4F(app::getSprite(getSprite()).color),
		0.25
	);
}

CatadioptricBullet::CatadioptricBullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes,
	shared_ptr<bullet_properties> props
) :
	BulletImpl(params, attributes, props)
{}

shared_ptr<LightArea> CatadioptricBullet::getLightSource() const {
	return CircleLightArea::create(
		getPos(),
		props->dimensions.x * 4.0f,
		Color4F::BLUE*0.5f + Color4F::GREEN*0.5f,
		0.25
	);
}

const string FlanPolarBullet::props = "flandrePolarBullet";
const SpaceFloat FlanPolarBullet::A = 5.0;
const SpaceFloat FlanPolarBullet::B = 4.0;
const SpaceFloat FlanPolarBullet::W = 1.0;

SpaceVect FlanPolarBullet::parametric_motion(SpaceFloat t)
{
	SpaceFloat theta = t * W;
	SpaceFloat r = A * cos(B*theta);
	return SpaceVect::ray(r, theta);
}

FlanPolarBullet::FlanPolarBullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes
) :
	BulletImpl(params, attributes, app::getBullet(props))
{
	ignoreObstacleCollision = true;
}

const string FlandrePolarMotionOrb::props = "flandrePolarMotionOrb";

FlandrePolarMotionOrb::FlandrePolarMotionOrb(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes
) :
	BulletImpl(params, attributes, app::getBullet(props))
{}

void FlandrePolarMotionOrb::update()
{
	GObject::update();

	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(app::params.secondsPerFrame * float_pi);
}

Lavaeteinn::Lavaeteinn(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes
) :
	Bullet(
		params,
		attributes,
		physics_params(SpaceVect(2.0, 0.5), 0.1)
	)
{
	setShield(false);
}

DamageInfo Lavaeteinn::getDamageInfo() const {
	return melee_damage(5.0f);
}

//void Lavaeteinn::update()
//{
//	GObject::update();
//
//	if (drawNodeID != 0) {
//		space->setSpriteAngle(drawNodeID, -toDegrees(getAngle()));
//	}
//}
//
//void Lavaeteinn::initializeGraphics()
//{
//	ImageSprite::initializeGraphics();
//
//	SpaceVect _dim = getDimensions();
//	float hWidth = to_float(_dim.x / 2.0 * App::pixelsPerTile);
//	float hHeight = to_float(_dim.y / 2.0 * App::pixelsPerTile);
//
//	drawNodeID = space->createDrawNode(GraphicsLayer::agentOverlay, getInitialCenterPix(), 1.0f);
//	space->drawSolidRect(drawNodeID, Vec2(-hWidth, -hHeight), Vec2(hWidth, hHeight), Color4F(.66f, .75f, .66f, .7f));
//}

FlandreCounterClockBullet::FlandreCounterClockBullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes
) :
	Bullet(
		params,
		attributes,
		physics_params(SpaceVect(4.0, 0.5), 0.1)
	)
{
	setShield(true);
}

DamageInfo FlandreCounterClockBullet::getDamageInfo() const {
	return melee_damage(1.5f);
}

CirnoIceShieldBullet::CirnoIceShieldBullet(
	shared_ptr<object_params> params,
	const bullet_attributes& attributes
) :
	Bullet(
		params,
		attributes,
		physics_params(0.3, 0.1)
	)
{
	setShield(true);
}

DamageInfo CirnoIceShieldBullet::getDamageInfo() const {
	return 	DamageInfo{ 10.0f, Attribute::iceDamage, DamageType::bullet };
}
