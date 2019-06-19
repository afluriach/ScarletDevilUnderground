//
//  EnemyBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "app_constants.hpp"
#include "AIUtil.hpp"
#include "EnemyBullet.hpp"
#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"
#include "MiscMagicEffects.hpp"

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    BulletImpl(&props) \
{} \

EnemyBulletImpl::EnemyBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	BulletImpl(props)
{}


FireBullet::FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	MaxSpeedImpl(speed)
{}

WaterBullet::WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	MaxSpeedImpl(speed)
{}

const vector<string> StarBullet::colors = {
	"blue",
	"green",
	"grey",
	"indigo",
	"purple",
	"red",
	"yellow"
};

StarBullet::StarBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed, SpaceFloat radius, const string& color) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	MaxSpeedImpl(speed),
	color(color)
{}

DamageInfo StarBullet::getDamageInfo() const {
	return bullet_damage(1.0f);
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, object_ref<Agent> agent, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(space,id,"", pos, 0.0),
Bullet(agent),
RectangleBody(SpaceVect(0.8, 0.175))
{
    setInitialAngularVelocity(angular_velocity);
}

SpaceFloat IllusionDialDagger::targetViewAngle()
{
    GObject* target = space->getObject("player");

    if(target)
        return ai::viewAngleToTarget(this,target);
    else
        return numeric_limits<SpaceFloat>::infinity();
}

void IllusionDialDagger::launch()
{
    GObject* target = space->getObject("player");

    if(target){
        setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
        setAngularVel(0.0f);
    }
    else
        debug_log("player missing");
}

void IllusionDialDagger::update()
{
	GObject::update();

	if (drawNodeID != 0) {
		space->graphicsNodeAction(
			&Node::setRotation,
			drawNodeID,
			to_float(-toDegrees(getAngle()))
		);
	}
}

void IllusionDialDagger::initializeGraphics()
{
	ImageSprite::initializeGraphics();

	SpaceVect _dim = getDimensions();
	float hWidth = to_float(_dim.x / 2.0 * app::pixelsPerTile);
	float hHeight = to_float(_dim.y / 2.0 * app::pixelsPerTile);
	
	drawNodeID = space->createSprite(
		&graphics_context::createDrawNode,
		GraphicsLayer::agentOverlay,
		getInitialCenterPix(),
		1.0f
	);
	space->graphicsNodeAction(
		&DrawNode::drawSolidRect,
		drawNodeID,
		Vec2(-hWidth, -hHeight),
		Vec2(hWidth, hHeight),
		Color4F(.66f, .75f, .66f, .7f)
	);
	space->graphicsNodeAction(&Node::setVisible, drawNodeID, false);
}

const bullet_properties ReimuBullet1::props = {
	0.1,
	4.5,
	0.2,
	bullet_damage(3.0f),
	"yinYangOrb",
};

const SpaceFloat ReimuBullet1::omega = float_pi * 2.0;
const SpaceFloat ReimuBullet1::amplitude = 2.0;

SpaceVect ReimuBullet1::parametric_move(SpaceFloat t, SpaceFloat firingAngle, SpaceFloat phaseAngleStart)
{
	SpaceVect d1 = SpaceVect::ray(t * props.speed, firingAngle);
	SpaceVect d2 = SpaceVect::ray(amplitude, firingAngle + float_pi * 0.5)*cos((t+phaseAngleStart)*omega);

	return d1 + d2;
}

ReimuBullet1::ReimuBullet1(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat start) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	BulletImpl(makeSharedCopy(props)),
	ParametricMotion(bind(&parametric_move, placeholders::_1, angle, start))
{}

void ReimuBullet1::update()
{
	ParametricMotion::_update();
}

const bullet_properties YinYangOrb::props = {
	0.1,
	4.5,
	0.5,
	bullet_damage(10.0f),
	"yinYangOrb",
	-1,
	-1,
};

YinYangOrb::YinYangOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	BulletImpl(makeSharedCopy(props))
{
	setInitialAngularVelocity(float_pi);
}

shared_ptr<LightArea> YinYangOrb::getLightSource() const
{
	return app::getLight("yinYangOrb");
}

const bullet_properties RumiaDemarcation2Bullet::props = {
	0.1,
	6.0,
	0.2,
	bullet_damage(7.5f),
	"rumiaDemarcationBullet",
	-1,
	0,
	false
};

RumiaDemarcation2Bullet::RumiaDemarcation2Bullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	object_ref<Agent> agent,
	SpaceFloat angularVel
) :
	GObject(space, id, "", pos, angle), 
	Bullet(agent),
	ShieldBullet(agent, false),
	BulletImpl(makeSharedCopy(props))
{
	setInitialAngularVelocity(angularVel);
}

void RumiaDemarcation2Bullet::update()
{
	GObject::update();

	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

const bullet_properties RumiaDarknessBullet::props = {
	0.1,
	6.0,
	0.2,
	DamageInfo{15.0f,Attribute::darknessDamage,DamageType::bullet},
	"",
	-1,
	0,
	true
};

RumiaDarknessBullet::RumiaDarknessBullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	object_ref<Agent> agent
) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	ShieldBullet(agent, false),
	BulletImpl(makeSharedCopy(props))
{
	addMagicEffect(make_shared<RadiusEffect>(
		this,
		DamageInfo{7.5f,Attribute::darknessDamage,DamageType::effectArea},
		2.0,
		GType::player
	));
}

shared_ptr<LightArea> RumiaDarknessBullet::getLightSource() const
{
	return app::getLight("rumiaDarknessBullet");
}
