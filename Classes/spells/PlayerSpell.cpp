//
//  PlayerSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "graphics_context.hpp"
#include "Player.hpp"
#include "PlayerSpell.hpp"

const string PlayerBatMode::name = "PlayerBatMode";
const string PlayerBatMode::description = "";
const string PlayerBatMode::icon = "sprites/ui/bat_mode.png";
const spell_cost PlayerBatMode::cost = spell_cost::ongoingMP(5.0f);

PlayerBatMode::PlayerBatMode(GObject* caster) :
	Spell(caster, spell_params{ -1.0, -1.0, PlayerBatMode::cost })
{}

void PlayerBatMode::init()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->setSprite("flandre_bat");
		p->setSpriteZoom(4.0f);

		p->setFiringSuppressed(true);
		p->modifyAttribute(Attribute::agility, 1.5f);
		p->setLayers(PhysicsLayers::ground);
		p->setProtection();
	}
}

void PlayerBatMode::end()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->setSprite("flandre");
		p->setSpriteZoom(1.0f);

		p->setFiringSuppressed(false);
		p->modifyAttribute(Attribute::agility, -1.5f);
		p->setLayers(enum_bitwise_or(PhysicsLayers, floor, ground));
		p->resetProtection();
	}
}

const string LavaeteinnSpell::name = "LavaeteinnSpell";
const string LavaeteinnSpell::description = "";
const string LavaeteinnSpell::icon = "sprites/ui/lavaeteinn.png";
const spell_cost LavaeteinnSpell::cost = spell_cost::initialStamina(12.5f);

const SpaceFloat LavaeteinnSpell::length = 2.0 / 3.0;
const SpaceFloat LavaeteinnSpell::angleWidth = float_pi / 2.0;
const SpaceFloat LavaeteinnSpell::angular_speed = angleWidth * 2.0 / length;
const int LavaeteinnSpell::bulletSpawnCount = 8;

LavaeteinnSpell::LavaeteinnSpell(GObject* caster) :
	Spell(caster, spell_params{ LavaeteinnSpell::length, 0.0, LavaeteinnSpell::cost })
{}

void LavaeteinnSpell::init()
{
	auto props = app::getBullet("lavaeteinn");
	SpaceFloat angle = canonicalAngle(caster->getAngle() - angleWidth);
	speedScale = getCasterAs<Agent>()->getAttribute(Attribute::attackSpeed);

	lavaeteinnBullet = getCasterAs<Agent>()->spawnBullet(
		props,
		SpaceVect::ray(1.5, angle),
		SpaceVect::zero,
		angle,
		angular_speed * speedScale
	);

	fireTimer = length / bulletSpawnCount;
	angularPos = angle;
}

void LavaeteinnSpell::update()
{
	timerDecrement(fireTimer);
	timerIncrement(angularPos, angular_speed * speedScale);

	if (lavaeteinnBullet.isValid()) {
		lavaeteinnBullet.get()->setPos(caster->getPos() + SpaceVect::ray(1.5, angularPos));
	}

	if (fireTimer <= 0.0) {
		auto props = app::getBullet("flandreFastOrb1");
		getCasterAs<Agent>()->launchBullet(props, SpaceVect::ray(2.0, angularPos), angularPos);
		fireTimer = length / bulletSpawnCount;
	}
}

void LavaeteinnSpell::end()
{
	if (lavaeteinnBullet.isValid()) {
		getSpace()->removeObject(lavaeteinnBullet);
	}
}

const string PlayerCounterClock::name = "PlayerCounterClock";
const string PlayerCounterClock::description = "";
const string PlayerCounterClock::icon = "sprites/ui/counterclock.png";
const spell_cost PlayerCounterClock::cost = spell_cost::ongoingMP(7.5f);

const SpaceFloat PlayerCounterClock::offset = 0.75;
const SpaceFloat PlayerCounterClock::angular_speed = 9.0;

PlayerCounterClock::PlayerCounterClock(GObject* caster) :
	Spell(caster, spell_params{ -1.0, 0.0, PlayerCounterClock::cost })
{}

void PlayerCounterClock::init()
{
	Player* p = getCasterAs<Player>();
	SpaceVect pos = caster->getPos();
	auto props = app::getBullet("flandreCounterClockBullet");

	if (p) {
		p->setFiringSuppressed(true);
	}

	for_irange(i, 0, 4)
	{
		SpaceVect disp = SpaceVect::ray(2.0 + offset, (i/2.0) * float_pi);

		bullets[i] = p->spawnBullet(props, disp, SpaceVect::zero, (i / 2.0) * float_pi, 0.0);
	}
}

void PlayerCounterClock::update()
{
	angular_pos += angular_speed * app::params.secondsPerFrame;

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
	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(false);
	}

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			getSpace()->removeObject(ref);
		}
	}
}

const string PlayerScarletRose::name = "PlayerScarletRose";
const string PlayerScarletRose::description = "";
const string PlayerScarletRose::icon = "sprites/ui/scarlet_rose.png";
const spell_cost PlayerScarletRose::cost = spell_cost::initialMP(15.0f);

const SpaceFloat PlayerScarletRose::fireInterval = 0.2;
const int PlayerScarletRose::fireCount = 6;

const SpaceFloat PlayerScarletRose::A = 5.0;
const SpaceFloat PlayerScarletRose::B = 4.0;
const SpaceFloat PlayerScarletRose::W = 1.0;

SpaceVect PlayerScarletRose::parametric_motion(SpaceFloat t)
{
	SpaceFloat theta = t * W;
	SpaceFloat r = A * cos(B*theta);
	return SpaceVect::ray(r, theta);
}

PlayerScarletRose::PlayerScarletRose(GObject* caster) :
	Spell(caster, spell_params{ 4.0, fireInterval, PlayerScarletRose::cost }),
	origin(caster->getPos())
{
	props = app::getBullet("flandrePolarBullet");
}

void PlayerScarletRose::update()
{
	if (launchCount < fireCount) {
		for_irange(i, 0, 8) {
			SpaceFloat t = float_pi / B * i;
			gobject_ref ref = getCasterAs<Agent>()->spawnBullet(
				props,
				SpaceVect::zero,
				SpaceVect::zero,
				0.0,
				0.0
			);

			parametric_space_function f = [t, this](SpaceFloat _t)->SpaceVect {
				return this->origin + parametric_motion(t + _t);
			};

			caster->makeInitMessage(&GObject::setParametricMove, ref, f, parametric_type::position );
		}

		timer -= fireInterval;
		++launchCount;
	}
}

void PlayerScarletRose::end()
{
	for (auto ref : bullets) {
		if (ref.isValid())
			getSpace()->removeObject(ref);
	}
}

const string PlayerIceShield::name = "PlayerIceShield";
const string PlayerIceShield::description = "";
const string PlayerIceShield::icon = "sprites/ui/ice_shield.png";
const spell_cost PlayerIceShield::cost = spell_cost::initialMP(12.5f);

const SpaceFloat PlayerIceShield::speed = 9.0;

const SpaceFloat PlayerIceShield::distance = 3.0;
const SpaceFloat PlayerIceShield::circumference = 2.0 * float_pi * distance;
const SpaceFloat PlayerIceShield::inv_circumference = 1.0 / circumference;

PlayerIceShield::PlayerIceShield(GObject* caster) :
	Spell(caster, spell_params{ -1.0, 0.0, PlayerIceShield::cost })
{
	props = app::getBullet("cirnoIceShieldBullet");
}

void PlayerIceShield::init()
{
	SpaceVect origin = caster->getPos();

	for_irange(i,0,bulletCount)
	{
		SpaceFloat angle = (1.0 * i / bulletCount) * (float_pi * 2.0);

		bullets[i] = getCasterAs<Agent>()->spawnBullet(
			props,
			SpaceVect::ray(distance, angle),
			SpaceVect::zero,
			angle - float_pi * 0.5,
			0.0
		);		
	}
}

void PlayerIceShield::update()
{
	//Update angle based on linear speed
	SpaceFloat dp = app::params.secondsPerFrame * speed;
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
	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			getSpace()->removeObject(ref);
		}
	}
}

