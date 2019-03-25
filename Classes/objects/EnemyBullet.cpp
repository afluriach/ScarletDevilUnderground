//
//  EnemyBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AIUtil.hpp"
#include "App.h"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    EnemyBullet(agent), \
    BulletImpl(&props) \
{} \

EnemyBullet::EnemyBullet(object_ref<Agent> agent) :
	Bullet(agent)
{}

const bullet_properties EnemyBulletImpl::fairy1Bullet = {
	0.1,
	6.0,
	0.3,
	hp_damage_map(5.0f),
	0.83,
	"sprites/ice_fairy_bullet.png",
};

const bullet_properties EnemyBulletImpl::greenFairyBullet = {
	0.1,
	6.0,
	0.15,
	hp_damage_map(3.0f),
	0.83,
	"sprites/green_fairy_bullet.png",
};

const bullet_properties EnemyBulletImpl::blueFairyBullet = {
	3.0,
	4.0,
	0.25,
	hp_damage_map(5.0f),
	0.83,
	"sprites/blue_fairy_bullet.png",
};

const bullet_properties EnemyBulletImpl::iceFairyBullet = {
	0.1,
	6.0,
	0.3,
	{ { Attribute::hp, -1 },{ Attribute::iceDamage, 25 } },
	0.83,
	"sprites/ice_fairy_bullet.png",
};

const bullet_properties EnemyBulletImpl::launcherBullet = {
	0.1,
	10.0,
	0.3,
	hp_damage_map(1.0f),
	0.83,
	"sprites/launcher_bullet.png",
};

const bullet_properties EnemyBulletImpl::rumiaBullet = {
	0.1,
	4.5,
	0.15,
	hp_damage_map(5.0f),
	0.83,
	"sprites/rumia_bullet.png",
};

const bullet_properties EnemyBulletImpl::rumiaDemarcationBullet = {
	0.1,
	4.5,
	0.1,
	hp_damage_map(5.0f),
	0.83,
	"sprites/rumia_demarcation_bullet.png",
};

const bullet_properties EnemyBulletImpl::rumiaPinwheelBullet = {
	0.1,
	4.5,
	0.1,
	hp_damage_map(2.0f),
	0.83,
	"sprites/rumia_bullet.png",
	Color3B::WHITE,
	1,
	0,
	false
};

EnemyBulletImpl::EnemyBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, const bullet_properties* props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	BulletImpl(props)
{}


FireBullet::FireBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	MaxSpeedImpl(speed)
{}

WaterBullet::WaterBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, SpaceFloat speed) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
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
	EnemyBullet(agent),
	MaxSpeedImpl(speed),
	color(color)
{}

AttributeMap StarBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

IllusionDialDagger::IllusionDialDagger(GSpace* space, ObjectIDType id, object_ref<Agent> agent, const SpaceVect& pos, SpaceFloat angular_velocity) :
GObject(space,id,"", pos, 0.0),
Bullet(agent),
EnemyBullet(agent),
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
		space->setSpriteAngle(drawNodeID, -toDegrees(getAngle()));
	}
}

void IllusionDialDagger::initializeGraphics()
{
	ImageSprite::initializeGraphics();

	SpaceVect _dim = getDimensions();
	float hWidth = to_float(_dim.x / 2.0 * App::pixelsPerTile);
	float hHeight = to_float(_dim.y / 2.0 * App::pixelsPerTile);
	
	drawNodeID = space->createDrawNode(GraphicsLayer::agentOverlay, getInitialCenterPix(), 1.0f);
	space->drawSolidRect(drawNodeID, Vec2(-hWidth, -hHeight), Vec2(hWidth, hHeight), Color4F(.66f, .75f, .66f, .7f));
	space->setSpriteVisible(drawNodeID, false);
}

const bullet_properties ReimuBullet1::props = {
	0.1,
	4.5,
	0.2,
	hp_damage_map(3.0f),
	0.5,
	"sprites/yin-yang-orb.png",
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
	EnemyBullet(agent),
	BulletImpl(&props),
	ParametricMotion(bind(&parametric_move, placeholders::_1, angle, start))
{}

void ReimuBullet1::update()
{
	updateSprite();
	ParametricMotion::_update();
}

const bullet_properties YinYangOrb::props = {
	0.1,
	4.5,
	0.5,
	hp_damage_map(10.0f),
	0.5,
	"sprites/yin-yang-orb.png",
	Color3B::BLACK,
	-1,
	-1,
};

YinYangOrb::YinYangOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	EnemyBullet(agent),
	BulletImpl(&props)
{
	knockback = 150.0;

	setInitialAngularVelocity(float_pi);
}

SpriteLightArea YinYangOrb::getLightSource() const
{
	return SpriteLightArea{
		getPos(),
		imageSpritePath(),
		Color4F(0.5f, 0.25f, 0.25f, 1.0f),
		3.0f
	};
}

const bullet_properties RumiaDemarcation2Bullet::props = {
	0.1,
	6.0,
	0.2,
	hp_damage_map(7.5f),
	0.83,
	"sprites/rumia_demarcation_bullet.png",
};

RumiaDemarcation2Bullet::RumiaDemarcation2Bullet(
	GSpace* space,
	ObjectIDType id,
	const SpaceVect& pos,
	SpaceFloat angle,
	object_ref<Agent> agent,
	SpaceFloat angularVel,
	SpaceFloat ttl
) :
	GObject(space, id, "", pos, angle), 
	Bullet(agent),
	EnemyBullet(agent),
	BulletImpl(&props),
	ttl(ttl)
{
	setInitialAngularVelocity(angularVel);
}

void RumiaDemarcation2Bullet::update()
{
	GObject::update();

	setVel(SpaceVect::ray(getMaxSpeed(), getAngle()));

	timerDecrement(ttl);
	if (ttl <= 0.0) {
		space->removeObject(this);
	}
}
