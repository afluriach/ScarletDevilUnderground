//
//  PlayerBullet.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "PlayerBullet.hpp"

void PlayerShield::onWallCollide(Wall* wall)
{
	//NO-OP
}

void PlayerShield::onEnvironmentCollide(GObject* obj)
{
	//NO-OP
}

void PlayerShield::onAgentCollide(Agent* agent)
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

FlandreBigOrb1::FlandreBigOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle)
{}

AttributeMap FlandreBigOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -3 }
	};
}

FlandreFastOrb1::FlandreFastOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle)
{}

AttributeMap FlandreFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

FlandreCounterClockBullet::FlandreCounterClockBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle),
	RectangleBody(SpaceVect(4.0, 0.5))
{}

AttributeMap FlandreCounterClockBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

RumiaFastOrb1::RumiaFastOrb1(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle)
{}

AttributeMap RumiaFastOrb1::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 }
	};
}

CirnoSmallIceBullet::CirnoSmallIceBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle)
{}

void CirnoSmallIceBullet::onAgentCollide(Agent* agent)
{
	agent->hit(getAttributeEffect(), getMagicEffect(agent));
	
	--hitsRemaining;

	if (hitsRemaining <= 0) {
		space->removeObject(this);
	}
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

CirnoLargeIceBullet::CirnoLargeIceBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space,id,"", pos, angle)
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

CirnoIceShieldBullet::CirnoIceShieldBullet(GSpace* space, ObjectIDType id, SpaceFloat angle, const SpaceVect& pos) :
	GObject(space, id, "", pos, angle)
{}

AttributeMap CirnoIceShieldBullet::getAttributeEffect() const {
	return {
		{ Attribute::hp, -1 },
		{ Attribute::iceDamage, 50 }
	};
}
