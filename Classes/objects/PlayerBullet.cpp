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
    GObject(make_shared<object_params>(space,id,"", pos, angle)), \
    Bullet(agent), \
    BulletImpl(app::getBullet(props)) \
{} \

PlayerBulletImpl::PlayerBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
    BulletImpl(props)
{}


StarbowBreakBullet::StarbowBreakBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
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
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
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

const string ScarletDagger::props = "scarletDagger";

ScarletDagger::ScarletDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	RectangleBody(SpaceVect(0.8, 0.175))
{}

shared_ptr<LightArea> ScarletDagger::getLightSource() const {
	return app::getLight("scarletDagger");
};

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

FlanPolarBullet::FlanPolarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	FlanPolarBullet(space,id,pos,angle,agent, 0.0)
{}

FlanPolarBullet::FlanPolarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat parametric_start) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	BulletImpl(app::getBullet(props)),
	ParametricMotion(&parametric_motion, parametric_start)
{
	ignoreObstacleCollision = true;
}

void FlanPolarBullet::update()
{
	GObject::update();
	ParametricMotion::_update();
}

const string FlandrePolarMotionOrb::props = "flandrePolarMotionOrb";

FlandrePolarMotionOrb::FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	BulletImpl(app::getBullet(props))
{}

void FlandrePolarMotionOrb::update()
{
	GObject::update();

	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(app::params.secondsPerFrame * float_pi);
}

Lavaeteinn::Lavaeteinn(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	SpaceFloat angularVel,
	object_ref<Agent> agent
) :
	GObject(make_shared<object_params>(space,id,"", pos, angle)),
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
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	ShieldBullet(agent, true),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

DamageInfo FlandreCounterClockBullet::getDamageInfo() const {
	return melee_damage(1.5f);
}

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	CircleBody(0.3),
	Bullet(agent),
	ShieldBullet(agent, true)
{}

DamageInfo CirnoIceShieldBullet::getDamageInfo() const {
	return 	DamageInfo{ 10.0f, Attribute::iceDamage, DamageType::bullet };
}
