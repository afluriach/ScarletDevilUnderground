//
//  PlayerSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Player.hpp"
#include "PlayerBullet.hpp"
#include "PlayerSpell.hpp"

void PlayerSpell::init()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->setTimedProtection(getLength());
	}
}

void PlayerSpell::update()
{
	timerIncrement(timeInSpell);

	if (timeInSpell > getLength()) {
		active = false;
	}
}

void PlayerSpell::end()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->resetProtection();
		p->onSpellStop();
	}
}

const string PlayerBatMode::name = "PlayerBatMode";
const string PlayerBatMode::description = "";

PlayerBatMode::PlayerBatMode(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerBatMode::init()
{
	PlayerSpell::init();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(true);
		p->setSprite("flandre_bat");
		p->applyAttributeModifier(Attribute::speed, 1.5f);
		p->setLayers(PhysicsLayers::ground);
	}
}

void PlayerBatMode::end()
{
	PlayerSpell::end();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(false);
		p->setSprite("flandre");
		p->applyAttributeModifier(Attribute::speed, -1.5f);
		p->setLayers(enum_bitwise_or(PhysicsLayers, floor, ground));
	}
}

const string PlayerCounterClock::name = "PlayerCounterClock";
const string PlayerCounterClock::description = "";

const SpaceFloat PlayerCounterClock::offset = 0.75;
const SpaceFloat PlayerCounterClock::angular_speed = 9.0;

PlayerCounterClock::PlayerCounterClock(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerCounterClock::init()
{
	PlayerSpell::init();

	Player* p = getCasterAs<Player>();
	SpaceVect pos = caster->getPos();

	if (p) {
		p->setFiringSuppressed(true);
	}

	for_irange(i, 0, 4)
	{
		SpaceVect disp = SpaceVect::ray(2.0 + offset, (i/2.0) * float_pi);

		bullets[i] = caster->space->createObject(GObject::make_object_factory<FlandreCounterClockBullet>(
			(i / 2.0) * float_pi,
			pos + disp
		));
	}
}

void PlayerCounterClock::update()
{
	PlayerSpell::update();
	angular_pos += angular_speed * App::secondsPerFrame;

	SpaceVect pos = caster->getPos();

	for_irange(i, 0, 4)
	{
		SpaceVect disp = SpaceVect::ray(2.0 + offset, (i / 2.0) * float_pi + angular_pos);

		if (bullets[i].isValid()) {
			bullets[i].get()->setPos(pos + disp);
			bullets[i].get()->setAngle((i / 2.0) * float_pi + angular_pos);
		}
	}
}

void PlayerCounterClock::end()
{
	PlayerSpell::end();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(false);
	}

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			caster->space->removeObject(ref.get());
		}
	}
}

const string PlayerDarkMist::name = "PlayerDarkMist";
const string PlayerDarkMist::description = "";

PlayerDarkMist::PlayerDarkMist(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerDarkMist::init()
{
	PlayerSpell::init();

	caster->sprite->setOpacity(128);
	caster->setInvisible(true);
}

void PlayerDarkMist::end()
{
	PlayerSpell::end();

	caster->sprite->setOpacity(255);
	caster->setInvisible(false);
}

const string PlayerIceShield::name = "PlayerIceShield";
const string PlayerIceShield::description = "";

const SpaceFloat PlayerIceShield::speed = 9.0;

const SpaceFloat PlayerIceShield::distance = 3.0;
const SpaceFloat PlayerIceShield::circumference = 2.0 * float_pi * distance;
const SpaceFloat PlayerIceShield::inv_circumference = 1.0 / circumference;

PlayerIceShield::PlayerIceShield(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerIceShield::init()
{
	PlayerSpell::init();

	SpaceVect origin = caster->getPos();

	for_irange(i,0,bulletCount)
	{
		SpaceFloat angle = (1.0 * i / bulletCount) * (float_pi * 2.0);
		SpaceVect pos = SpaceVect::ray(distance, angle);
		
		bullets[i] = caster->space->createObject(
			GObject::make_object_factory<CirnoIceShieldBullet>(
				angle - float_pi / 2.0,
				origin + pos
			)
		);
	}
}

void PlayerIceShield::update()
{
	PlayerSpell::update();

	//Update angle based on linear speed
	SpaceFloat dp = App::secondsPerFrame * speed;
	crntAngle += dp * inv_circumference * float_pi * 2.0;

	if (crntAngle >= 2.0*float_pi) {
		crntAngle -= 2.0*float_pi;
	}

	SpaceVect origin = caster->getPos();

	for_irange(i, 0, bulletCount)
	{
		SpaceFloat angle = (1.0 * i / bulletCount) * (float_pi * 2.0) + crntAngle;
		SpaceVect pos = SpaceVect::ray(distance, angle);
		SpaceVect vel = SpaceVect::ray(speed, angle - float_pi/2.0); 

		if (bullets[i].isValid()) {
			bullets[i].get()->setPos(origin + pos);
			bullets[i].get()->setVel(vel);
			bullets[i].get()->setAngle(angle);
		}
	}
}

void PlayerIceShield::end()
{
	PlayerSpell::end();

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			caster->space->removeObject(ref.get());
		}
	}
}

