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

FlandreBigOrb1::FlandreBigOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle),
	PlayerBullet(agent)
{}

AttributeMap FlandreBigOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -3 }
	};
}

FlandreFastOrb1::FlandreFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle),
	PlayerBullet(agent)
{
}

AttributeMap FlandreFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

FlandrePolarMotionOrb::FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	PlayerBullet(agent),
	RegisterUpdate<FlandrePolarMotionOrb>(this)
{}

void FlandrePolarMotionOrb::update()
{
	applyForceForSingleFrame(SpaceVect::ray(20.0, getAngle() + float_pi / 2.0));
	rotate(App::secondsPerFrame * float_pi);
}

AttributeMap FlandrePolarMotionOrb::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

AttributeMap FlandreCounterClockBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

RumiaFastOrb1::RumiaFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	PlayerBullet(agent)
{}

AttributeMap RumiaFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

CirnoSmallIceBullet::CirnoSmallIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	PlayerBullet(agent)
{
	hitCount = 3;
}

AttributeMap CirnoSmallIceBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}

shared_ptr<MagicEffect> CirnoSmallIceBullet::getMagicEffect(gobject_ref target)
{
	return nullptr;
}

CirnoLargeIceBullet::CirnoLargeIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle),
	PlayerBullet(agent)
{}

AttributeMap CirnoLargeIceBullet::getAttributeEffect() const {
	return {
		{Attribute::hp, -1},
		{Attribute::iceDamage, 50}
	};
}

shared_ptr<MagicEffect> CirnoLargeIceBullet::getMagicEffect(gobject_ref target)
{
	return nullptr;
}

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	PlayerShield(agent)
{}

AttributeMap CirnoIceShieldBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}
