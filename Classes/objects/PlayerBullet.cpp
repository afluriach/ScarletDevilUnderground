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
