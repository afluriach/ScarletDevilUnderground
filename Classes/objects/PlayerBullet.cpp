//
//  PlayerBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "Player.hpp"
#include "PlayerBullet.hpp"

#define cons(x) x::x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) : \
    GObject(space, id, "", pos, angle), \
    Bullet(agent), \
    PlayerBullet(agent), \
    BulletImpl(&props) \
{} \

PlayerBullet::PlayerBullet(Agent* agent) :
	Bullet(agent)
{
	ricochetCount = agent->getAttribute(Attribute::ricochet);
}

void PlayerBullet::onAgentCollide(Agent* agent, SpaceVect n)
{
	Bullet::onAgentCollide(agent, n);
	Player* p = space->getObjectAs<Player>("player");
	p->applyCombo(6);
}

PlayerShield::PlayerShield(Agent* agent) :
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

const bullet_properties FlandreBigOrb1::props = {
	0.1,
	4.5,
	0.6,
	0.83,
	"sprites/flandre_bullet.png",
	hp_damage_map(3.0f)
};

cons(FlandreBigOrb1)

const bullet_properties FlandreFastOrb1::props = {
	0.1,
	9.0,
	0.15,
	0.83,
	"sprites/flandre_bullet.png",
	hp_damage_map(1.0f)
};

cons(FlandreFastOrb1)

const bullet_properties FlandrePolarMotionOrb::props = {
	1.0,
	9.0,
	0.15,
	0.83,
	"sprites/flandre_bullet.png",
	hp_damage_map(1.0f)
};

FlandrePolarMotionOrb::FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBullet(agent),
	BulletImpl(&props),
	RegisterUpdate<FlandrePolarMotionOrb>(this)
{}

void FlandrePolarMotionOrb::update()
{
	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(App::secondsPerFrame * float_pi);
}

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

AttributeMap FlandreCounterClockBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

const bullet_properties RumiaFastOrb1::props = {
	0.1,
	9.0,
	0.15,
	0.83,
	"sprites/rumia_bullet.png",
	hp_damage_map(1.0f)
};

cons(RumiaFastOrb1)

const bullet_properties CirnoSmallIceBullet::props = {
	0.1,
	9.0,
	0.3,
	0.83,
	"sprites/cirno_large_ice_bullet.png",
	{ { Attribute::hp, -1 },{ Attribute::iceDamage, 50 } }
};

CirnoSmallIceBullet::CirnoSmallIceBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	Bullet(agent),
	PlayerBullet(agent),
	BulletImpl(&props)
{
	hitCount = 3;
}

const bullet_properties CirnoLargeIceBullet::props = {
	0.1,
	9.0,
	0.6,
	0.83,
	"sprites/cirno_large_ice_bullet.png",
	{ { Attribute::hp, -1 }, { Attribute::iceDamage, 50 } }
};

cons(CirnoLargeIceBullet)

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent)
{}

AttributeMap CirnoIceShieldBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}
