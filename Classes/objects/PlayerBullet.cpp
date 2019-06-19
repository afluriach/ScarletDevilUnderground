//
//  PlayerBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "GSpace.hpp"
#include "Player.hpp"
#include "PlayerBullet.hpp"

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    BulletImpl(makeSharedCopy(props)) \
{} \

const bullet_properties PlayerBulletImpl::flandreBigOrb1 = {
	0.1,
	4.5,
	0.6,
	bullet_damage(3.0f),
	"flandreOrb",
};

const bullet_properties PlayerBulletImpl::catadioptricBullet1 = {
	3.0,
	6.0,
	0.5,
	bullet_damage(3.0f),
	"catadioptricBullet1"
};

const bullet_properties PlayerBulletImpl::catadioptricBullet2 = {
	1.5,
	4.5,
	0.25,
	bullet_damage(1.5f),
	"catadioptricBullet2",
	1,
	1,
};

const bullet_properties PlayerBulletImpl::catadioptricBullet3 = {
	0.5,
	6.0,
	0.125,
	bullet_damage(0.5f),
	"catadioptricBullet3",
	1,
	2,
};

const bullet_properties PlayerBulletImpl::rumiaFastOrb1 = {
	0.1,
	9.0,
	0.15,
	bullet_damage(1.0f),
	"rumiaOrb",
};

const bullet_properties PlayerBulletImpl::cirnoSmallIceBullet = {
	0.1,
	9.0,
	0.3,
	DamageInfo{ 3.0f, Attribute::iceDamage, DamageType::bullet},
	"cirnoIceBullet",
	3
};

const bullet_properties PlayerBulletImpl::cirnoLargeIceBullet = {
	0.1,
	9.0,
	0.6,
	DamageInfo{ 5.0f, Attribute::iceDamage, DamageType::bullet },
	"cirnoIceBullet",
};

PlayerBulletImpl::PlayerBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
    GObject(space, id, "", pos, angle),
    Bullet(agent),
    BulletImpl(props)
{}


StarbowBreakBullet::StarbowBreakBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBulletImpl(space, id, pos, angle, agent, props)
{}

shared_ptr<LightArea> StarbowBreakBullet::getLightSource() const {
	return CircleLightArea::create(
		getPos(),
		props->radius * 4.0f,
		toColor4F(app::getSprite(getSprite()).color),
		0.25
	);
}

CatadioptricBullet::CatadioptricBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBulletImpl(space, id, pos, angle, agent, props)
{}

shared_ptr<LightArea> CatadioptricBullet::getLightSource() const {
	return CircleLightArea::create(
		getPos(),
		props->radius * 4.0f,
		Color4F::BLUE*0.5f + Color4F::GREEN*0.5f,
		0.25
	);
}

const bullet_properties ScarletDagger::props = {
	5.0,
	6.0,
	//unused, since it is not a CircleBody
	0.5,
	bullet_damage(5.0f),
	"scarletDagger",
};

ScarletDagger::ScarletDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	RectangleBody(SpaceVect(0.8, 0.175))
{}

shared_ptr<LightArea> ScarletDagger::getLightSource() const {
	return app::getLight("scarletDagger");
};

const bullet_properties FlandreFastOrb1::props = {
	0.1,
	9.0,
	0.15,
	bullet_damage(1.0f),
	"flandreOrb",
};

cons(FlandreFastOrb1)

shared_ptr<LightArea> FlandreFastOrb1::getLightSource() const {
	return app::getLight("flandreFastOrb");
}

const bullet_properties FlanPolarBullet::props = {
	0.1,
	0.0,
	0.15,
	bullet_damage(2.0f),
	"flandreOrb",
	-1,
	0,
	false
};

const SpaceFloat FlanPolarBullet::A = 5.0;
const SpaceFloat FlanPolarBullet::B = 4.0;
const SpaceFloat FlanPolarBullet::W = 1.0;

SpaceVect FlanPolarBullet::parametric_motion(SpaceFloat t)
{
	SpaceFloat theta = t * W;
	SpaceFloat r = A * cos(B*theta);
	return SpaceVect::ray(r, theta);
}

FlanPolarBullet::FlanPolarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	FlanPolarBullet(space,id,pos,angle,agent, 0.0)
{}

FlanPolarBullet::FlanPolarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat parametric_start) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	BulletImpl(makeSharedCopy(props)),
	ParametricMotion(&parametric_motion, parametric_start)
{
	ignoreObstacleCollision = true;
}

void FlanPolarBullet::update()
{
	GObject::update();
	ParametricMotion::_update();
}

shared_ptr<LightArea> FlanPolarBullet::getLightSource() const {
	return app::getLight("flandrePolarOrb");
}

const bullet_properties FlandrePolarMotionOrb::props = {
	1.0,
	9.0,
	0.15,
	bullet_damage(1.0f),
	"flandreOrb",
};

FlandrePolarMotionOrb::FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	BulletImpl(makeSharedCopy(props))
{}

void FlandrePolarMotionOrb::update()
{
	GObject::update();

	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(app::params.secondsPerFrame * float_pi);
}

shared_ptr<LightArea> FlandrePolarMotionOrb::getLightSource() const {
	return app::getLight("polarMotionOrb");
}

Lavaeteinn::Lavaeteinn(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	SpaceFloat angularVel,
	object_ref<Agent> agent
) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	ShieldBullet(agent, false),
	RectangleBody(SpaceVect(2.0, 0.5))
{
	setInitialAngularVelocity(angularVel);
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

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	ShieldBullet(agent, true),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

DamageInfo FlandreCounterClockBullet::getDamageInfo() const {
	return melee_damage(1.5f);
}

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	ShieldBullet(agent, true)
{}

DamageInfo CirnoIceShieldBullet::getDamageInfo() const {
	return 	DamageInfo{ 10.0f, Attribute::iceDamage, DamageType::bullet };
}
