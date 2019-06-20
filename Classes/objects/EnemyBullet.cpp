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
	GObject(make_shared<object_params>(space, id, "", pos, angle)), \
	Bullet(agent), \
    BulletImpl(&props) \
{} \

EnemyBulletImpl::EnemyBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	BulletImpl(props)
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
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	CircleBody(radius),
	MaxSpeedImpl(speed),
	color(color)
{}

DamageInfo StarBullet::getDamageInfo() const {
	return bullet_damage(1.0f);
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, object_ref<Agent> agent, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(make_shared<object_params>(space, id, "", pos, 0.0)),
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
	GObject::initializeGraphics();

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

const string ReimuBullet1::props = "reimuBullet1";
const SpaceFloat ReimuBullet1::omega = float_pi * 2.0;
const SpaceFloat ReimuBullet1::amplitude = 2.0;

SpaceVect ReimuBullet1::parametric_move(
	SpaceFloat t,
	SpaceFloat firingAngle,
	SpaceFloat phaseAngleStart,
	SpaceFloat speed
){
	SpaceVect d1 = SpaceVect::ray(t * speed, firingAngle);
	SpaceVect d2 = SpaceVect::ray(amplitude, firingAngle + float_pi * 0.5)*cos((t+phaseAngleStart)*omega);

	return d1 + d2;
}

ReimuBullet1::ReimuBullet1(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat start) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	BulletImpl(app::getBullet(props)),
	ParametricMotion(bind(&parametric_move, placeholders::_1, angle, start, app::getBullet(props)->speed))
{}

void ReimuBullet1::update()
{
	ParametricMotion::_update();
}

const string YinYangOrb::props = "yinYangOrb";

YinYangOrb::YinYangOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	BulletImpl(app::getBullet(props))
{
	setInitialAngularVelocity(float_pi);
}

const string RumiaDemarcation2Bullet::props = "rumiaDemarcationBullet2";

RumiaDemarcation2Bullet::RumiaDemarcation2Bullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	object_ref<Agent> agent,
	SpaceFloat angularVel
) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	ShieldBullet(agent, false),
	BulletImpl(app::getBullet(props))
{
	setInitialAngularVelocity(angularVel);
}

void RumiaDemarcation2Bullet::update()
{
	GObject::update();

	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));
}

const string RumiaDarknessBullet::props = "rumiaDarknessBullet";

RumiaDarknessBullet::RumiaDarknessBullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	object_ref<Agent> agent
) :
	GObject(make_shared<object_params>(space, id, "", pos, angle)),
	Bullet(agent),
	ShieldBullet(agent, false),
	BulletImpl(app::getBullet(props))
{
	addMagicEffect(make_shared<RadiusEffect>(
		this,
		DamageInfo{7.5f,Attribute::darknessDamage,DamageType::effectArea},
		2.0,
		GType::player
	));
}
