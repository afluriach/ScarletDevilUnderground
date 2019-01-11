//
//  PatchouliSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "EnemyBullet.hpp"
#include "GSpace.hpp"
#include "PatchouliSpell.hpp"

const string FireStarburst::name = "FireStarburst";
const string FireStarburst::description = "";

void FireStarburst::runPeriodic()
{
	SpaceVect pos = caster->body->getPos();
	for_irange(i, 0, 8)
	{
		SpaceFloat angle = float_pi * i / 4.0;

		SpaceVect crntPos = pos + SpaceVect::ray(1, angle);

		caster->space->createObject(
			GObject::make_object_factory<FireBullet>(angle, pos, bulletSpeed)
		);
	}
}

const string FlameFence::name = "FlameFence";
const string FlameFence::description = "";

FlameFence::FlameFence(GObject* caster) :
	Spell(caster, {}, Spell::getDescriptor("FlameFence").get())
{
}

void FlameFence::init()
{
	SpaceVect center = caster->body->getPos();

	for (int y = -10; y < 10; y += 2)
	{
		SpaceVect rowSkew(y % 2 ? 0.5 : 0, 0);
		for (int x = -10; x < 10; x += 2)
		{
			SpaceVect pos(center);
			pos += SpaceVect(x, y) + rowSkew;

			bullets.push_back(caster->space->createObject(
				GObject::make_object_factory<FireBullet>(0.0f, pos, 0.0f)
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
			caster->space->removeObject(bullet.get());
	}
}

const string Whirlpool1::name = "Whirlpool1";
const string Whirlpool1::description = "";

const int Whirlpool1::shotsPerSecond = 4;

const SpaceFloat Whirlpool1::angularSpeed = float_pi / 6.0;
const SpaceFloat Whirlpool1::angularOffset = float_pi / 12.0;
const SpaceFloat Whirlpool1::bulletSpeed = 6.0;

Whirlpool1::Whirlpool1(GObject* caster) :
	Spell(caster, {}, Spell::getDescriptor("Whirlpool1").get())
{
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
			caster->space->createObject(GObject::make_object_factory<WaterBullet>(
				angles[i],
				pos + SpaceVect::ray(1.0, angles[i]),
				bulletSpeed
				));
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool1::end()
{

}

const string Whirlpool2::name = "Whirlpool2";
const string Whirlpool2::description = "";

const int Whirlpool2::shotsPerSecond = 6;

const SpaceFloat Whirlpool2::angularSpeed = float_pi / 5.0;
const SpaceFloat Whirlpool2::angularOffset = float_pi / 10.0;
const SpaceFloat Whirlpool2::bulletSpeed = 7.5;

Whirlpool2::Whirlpool2(GObject* caster) :
	Spell(caster, {}, Spell::getDescriptor("Whirlpool2").get())
{
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
			caster->space->createObject(GObject::make_object_factory<WaterBullet>(
				angles[i],
				pos + SpaceVect::ray(1.0, angles[i]),
				bulletSpeed
				));
		}

		shotTimer = 1.0 / shotsPerSecond;
	}
}

void Whirlpool2::end()
{

}
