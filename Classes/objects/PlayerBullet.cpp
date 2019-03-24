//
//  PlayerBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "App.h"
#include "GSpace.hpp"
#include "Player.hpp"
#include "PlayerBullet.hpp"

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    PlayerBullet(agent), \
    BulletImpl(&props) \
{} \

PlayerBullet::PlayerBullet(object_ref<Agent> agent) :
	Bullet(agent)
{
	ricochetCount = agent.get()->getAttribute(Attribute::ricochet);
}

PlayerShield::PlayerShield(object_ref<Agent> agent) :
	Bullet(agent)
{
}

void PlayerShield::onWallCollide(Wall* wall)
{
	//NO-OP
}

void PlayerShield::onEnvironmentCollide(GObject* obj)
{
	//NO-OP
}

void PlayerShield::onAgentCollide(Agent* agent, SpaceVect n)
{
	agent->hit(getAttributeEffect(), getMagicEffect(agent));

	SpaceFloat force = getKnockbackForce();

	if (force > 0.0) {
		agent->applyImpulse(force, getAngle() + float_pi / 2.0);
	}
}

void PlayerShield::onBulletCollide(Bullet* bullet)
{
	space->removeObject(bullet);
}

const bullet_properties PlayerBulletImpl::flandreBigOrb1 = {
	0.1,
	4.5,
	0.6,
	hp_damage_map(3.0f),
	0.83,
	"sprites/flandre_bullet.png",
};

const bullet_properties PlayerBulletImpl::rumiaFastOrb1 = {
	0.1,
	9.0,
	0.15,
	hp_damage_map(1.0f),
	0.83,
	"sprites/rumia_bullet.png",
};

const bullet_properties PlayerBulletImpl::cirnoSmallIceBullet = {
	0.1,
	9.0,
	0.3,
	{ { Attribute::hp, -1 },{ Attribute::iceDamage, 50 } },
	0.83,
	"sprites/cirno_large_ice_bullet.png",
	Color3B::WHITE,
	3
};

const bullet_properties PlayerBulletImpl::cirnoLargeIceBullet = {
	0.1,
	9.0,
	0.6,
	{ { Attribute::hp, -1 },{ Attribute::iceDamage, 50 } },
	0.83,
	"sprites/cirno_large_ice_bullet.png",
};

PlayerBulletImpl::PlayerBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, const bullet_properties* props) :
    GObject(space, id, "", pos, angle),
    Bullet(agent),
    PlayerBullet(agent),
    BulletImpl(props)
{}

PlayerBulletValueImpl::PlayerBulletValueImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, bullet_properties props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBullet(agent),
	BulletValueImpl(props)
{}

StarbowBreakBullet::StarbowBreakBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, bullet_properties props) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBulletValueImpl(space, id, pos, angle, agent, props)
{}

const bullet_properties ScarletDagger::props = {
	5.0,
	4.0,
	//unused, since it is not a CircleBody
	0.5,
	hp_damage_map(3.0f),
	0.5,
	"sprites/scarlet_dagger.png",
};

ScarletDagger::ScarletDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBullet(agent),
	RectangleBody(SpaceVect(0.8, 0.175))
{}

const bullet_properties FlandreFastOrb1::props = {
	0.1,
	9.0,
	0.15,
	hp_damage_map(1.0f),
	0.83,
	"sprites/flandre_bullet.png",
};

cons(FlandreFastOrb1)

const bullet_properties FlanPolarBullet::props = {
	0.1,
	0.0,
	0.15,
	hp_damage_map(2.0f),
	0.83,
	"sprites/flandre_bullet.png",
	Color3B::WHITE,
	-1,
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
	PlayerBullet(agent),
	BulletImpl(&props),
	ParametricMotion(&parametric_motion, parametric_start)
{}

void FlanPolarBullet::update()
{
	updateSprite();
	ParametricMotion::_update();
}


CircleLightArea FlanPolarBullet::getLightSource() const {
	return CircleLightArea{ getPos(), 2.0, Color4F::ORANGE*0.5f, 0.0 }; 
}

const bullet_properties FlandrePolarMotionOrb::props = {
	1.0,
	9.0,
	0.15,
	hp_damage_map(1.0f),
	0.83,
	"sprites/flandre_bullet.png",
};

FlandrePolarMotionOrb::FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBullet(agent),
	BulletImpl(&props)
{}

void FlandrePolarMotionOrb::update()
{
	GObject::update();

	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(App::secondsPerFrame * float_pi);
}

CircleLightArea FlandrePolarMotionOrb::getLightSource() const {
	return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 };
}

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

AttributeMap FlandreCounterClockBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent)
{}

AttributeMap CirnoIceShieldBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}
