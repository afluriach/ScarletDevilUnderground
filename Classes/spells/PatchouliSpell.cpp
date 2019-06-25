//
//  PatchouliSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "PatchouliSpell.hpp"

const string FireStarburst::name = "FireStarburst";
const string FireStarburst::description = "";
const float FireStarburst::cost = 0.0f;

FireStarburst::FireStarburst(GObject* caster) : 
	PeriodicSpell(caster)
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("fireBullet");
	bulletProps->speed = bulletSpeed;
}

void FireStarburst::runPeriodic()
{
	SpaceVect pos = caster->getPos();
	for_irange(i, 0, 8)
	{
		SpaceFloat angle = float_pi * i / 4.0;

		SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

		getSpace()->createObject<BulletImpl>(
			Bullet::makeParams(pos,angle),
			getCasterAs<Agent>()->getBulletAttributes(bulletProps),
			bulletProps
		);
	}
}

const string FlameFence::name = "FlameFence";
const string FlameFence::description = "";
const float FlameFence::cost = 0.0f;

FlameFence::FlameFence(GObject* caster) :
	Spell(caster)
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("fireBullet");
	bulletProps->speed = 0.0;
	bulletProps->directionalLaunch = false;
}

void FlameFence::init()
{
	SpaceVect center = caster->getPos();

	for (int y = -10; y < 10; y += 2)
	{
		SpaceVect rowSkew(y % 2 ? 0.5 : 0, 0);
		for (int x = -10; x < 10; x += 2)
		{
			SpaceVect pos(center);
			pos += SpaceVect(x, y) + rowSkew;

			bullets.push_back(getSpace()->createObject<BulletImpl>(
				Bullet::makeParams(pos,0.0f),
				getCasterAs<Agent>()->getBulletAttributes(bulletProps),
				bulletProps
			));
		}
	}
}

void FlameFence::update()
{
}

void FlameFence::end()
{
	for (gobject_ref bullet : bullets) {
		if (bullet.isValid())
			getSpace()->removeObject(bullet);
	}
}

const string Whirlpool1::name = "Whirlpool1";
const string Whirlpool1::description = "";
const float Whirlpool1::cost = 0.0f;

const int Whirlpool1::shotsPerSecond = 4;

const SpaceFloat Whirlpool1::angularSpeed = float_pi / 6.0;
const SpaceFloat Whirlpool1::angularOffset = float_pi / 12.0;
const SpaceFloat Whirlpool1::bulletSpeed = 6.0;

Whirlpool1::Whirlpool1(GObject* caster) :
	Spell(caster)
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("waterBullet");
	bulletProps->speed = bulletSpeed;
}

void Whirlpool1::init()
{
	caster->setAngularVel(angularSpeed);
}

void Whirlpool1::update()
{
	timerDecrement(shotTimer);

	SpaceFloat angle = caster->getAngle();
	SpaceVect pos = caster->getPos();

	array<SpaceFloat, 6> angles;

	angles[0] = angle - angularOffset;
	angles[1] = angle;
	angles[2] = angle + angularOffset;

	angles[3] = float_pi + angle - angularOffset;
	angles[4] = float_pi + angle;
	angles[5] = float_pi + angle + angularOffset;

	if (shotTimer <= 0.0) {

		for_irange(i, 0, 6) {
			getSpace()->createObject<BulletImpl>(
				Bullet::makeParams(pos + SpaceVect::ray(1.0, angles[i]), angles[i]),
				getCasterAs<Agent>()->getBulletAttributes(bulletProps),
				bulletProps
			);
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool1::end()
{

}

const string Whirlpool2::name = "Whirlpool2";
const string Whirlpool2::description = "";
const float Whirlpool2::cost = 0.0f;

const int Whirlpool2::shotsPerSecond = 6;

const SpaceFloat Whirlpool2::angularSpeed = float_pi / 5.0;
const SpaceFloat Whirlpool2::angularOffset = float_pi / 10.0;
const SpaceFloat Whirlpool2::bulletSpeed = 7.5;

Whirlpool2::Whirlpool2(GObject* caster) :
	Spell(caster)
{
	bulletProps = make_shared<bullet_properties>();
	*bulletProps = *app::getBullet("waterBullet");
	bulletProps->speed = bulletSpeed;
}

void Whirlpool2::init()
{
	caster->setAngularVel(angularSpeed);
}

void Whirlpool2::update()
{
	timerDecrement(shotTimer);

	SpaceFloat angle = caster->getAngle();
	SpaceVect pos = caster->getPos();

	array<SpaceFloat, 12> angles;

	angles[0] = angle - angularOffset;
	angles[1] = angle;
	angles[2] = angle + angularOffset;

	angles[3] = (float_pi / 2.0) + angle - angularOffset;
	angles[4] = (float_pi / 2.0) + angle;
	angles[5] = (float_pi / 2.0) + angle + angularOffset;

	angles[6] = float_pi + angle - angularOffset;
	angles[7] = float_pi + angle;
	angles[8] = float_pi + angle + angularOffset;

	angles[9] = (float_pi * 1.5) + angle - angularOffset;
	angles[10] = (float_pi * 1.5) + angle;
	angles[11] = (float_pi * 1.5) + angle + angularOffset;


	if (shotTimer <= 0.0) {

		for_irange(i, 0, 12) {
			getSpace()->createObject<BulletImpl>(
				Bullet::makeParams(pos + SpaceVect::ray(1.0, angles[i]), angles[i]),
				getCasterAs<Agent>()->getBulletAttributes(bulletProps),
				bulletProps
			);
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool2::end()
{

}
