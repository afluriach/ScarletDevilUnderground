//
//  PlayerSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "graphics_context.hpp"
#include "GSpace.hpp"
#include "enum.h"
#include "Player.hpp"
#include "PlayerBullet.hpp"
#include "PlayerSpell.hpp"

void PlayerSpell::init()
{
}

void PlayerSpell::update()
{
	timerIncrement(timeInSpell);
	SpaceFloat length = getLength();

	if(length >= 0.0 && timeInSpell >= length) {
		active = false;
	}
}

void PlayerSpell::end()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->onSpellStop();
	}
}

const string PlayerBatMode::name = "PlayerBatMode";
const string PlayerBatMode::description = "";
const string PlayerBatMode::icon = "sprites/ui/bat_mode.png";
const float PlayerBatMode::cost = 5.0f;
const SpellCostType PlayerBatMode::costType = enum_bitwise_or(SpellCostType, mp, ongoing);


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
		p->space->addGraphicsAction(&graphics_context::setSpriteZoom, p->spriteID, 4.0f);
		p->applyAttributeModifier(Attribute::agility, 1.5f);
		p->setLayers(PhysicsLayers::ground);
		p->setProtection();
	}
}

void PlayerBatMode::end()
{
	PlayerSpell::end();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(false);
		p->setSprite("flandre");
		p->space->addGraphicsAction(&graphics_context::setSpriteZoom, p->spriteID, 1.0f);
		p->applyAttributeModifier(Attribute::agility, -1.5f);
		p->setLayers(enum_bitwise_or(PhysicsLayers, floor, ground));
		p->resetProtection();
	}
}

const string LavaeteinnSpell::name = "LavaeteinnSpell";
const string LavaeteinnSpell::description = "";
const string LavaeteinnSpell::icon = "sprites/ui/lavaeteinn.png";
const float LavaeteinnSpell::cost = 12.5f;
const SpellCostType LavaeteinnSpell::costType = enum_bitwise_or(SpellCostType, stamina, initial);

const SpaceFloat LavaeteinnSpell::length = 2.0 / 3.0;
const SpaceFloat LavaeteinnSpell::angleWidth = float_pi / 2.0;
const SpaceFloat LavaeteinnSpell::angular_speed = angleWidth * 2.0 / length;
const int LavaeteinnSpell::bulletSpawnCount = 8;

LavaeteinnSpell::LavaeteinnSpell(GObject* caster) :
	PlayerSpell(caster)
{}

void LavaeteinnSpell::init()
{
	PlayerSpell::init();

	SpaceFloat angle = canonicalAngle(caster->getAngle() - angleWidth);
	SpaceVect pos = caster->getPos() + SpaceVect::ray(1.5, angle);
	object_ref<Agent> agent = caster;
	speedScale = getCasterAs<Agent>()->getAttribute(Attribute::attackSpeed);

	lavaeteinnBullet = caster->space->createObject<Lavaeteinn>(
		pos,
		angle,
		angular_speed * speedScale,
		agent
	);

	fireTimer = length / bulletSpawnCount;
	angularPos = angle;
}

void LavaeteinnSpell::update()
{
	PlayerSpell::update();

	timerDecrement(fireTimer);
	timerIncrement(angularPos, angular_speed * speedScale);

	if (lavaeteinnBullet.isValid()) {
		lavaeteinnBullet.get()->setPos(caster->getPos() + SpaceVect::ray(1.5, angularPos));
	}

	if (fireTimer <= 0.0) {
		object_ref<Agent> agent = caster;
		caster->space->createObject<FlandreFastOrb1>(
			caster->getPos() + SpaceVect::ray(2.0, angularPos),
			angularPos,
			agent
		);
		fireTimer = length / bulletSpawnCount;
	}
}

void LavaeteinnSpell::end()
{
	PlayerSpell::end();

	if (lavaeteinnBullet.isValid()) {
		caster->space->removeObject(lavaeteinnBullet);
	}
}

const string PlayerCounterClock::name = "PlayerCounterClock";
const string PlayerCounterClock::description = "";
const string PlayerCounterClock::icon = "sprites/ui/counterclock.png";
const float PlayerCounterClock::cost = 7.5f;
const SpellCostType PlayerCounterClock::costType = enum_bitwise_or(SpellCostType, mp, ongoing);

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

		bullets[i] = caster->space->createObject<FlandreCounterClockBullet>(
			pos + disp,
			(i / 2.0) * float_pi,
			dynamic_cast<Agent*>(caster)
		);
	}
}

void PlayerCounterClock::update()
{
	PlayerSpell::update();
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
	PlayerSpell::end();

	Player* p = getCasterAs<Player>();

	if (p) {
		p->setFiringSuppressed(false);
	}

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			caster->space->removeObject(ref);
		}
	}
}

const string PlayerScarletRose::name = "PlayerScarletRose";
const string PlayerScarletRose::description = "";
const string PlayerScarletRose::icon = "sprites/ui/scarlet_rose.png";
const float PlayerScarletRose::cost = 15.0f;
const SpellCostType PlayerScarletRose::costType = enum_bitwise_or(SpellCostType, mp, initial);

const SpaceFloat PlayerScarletRose::fireInterval = 0.2;
const int PlayerScarletRose::fireCount = 6;

PlayerScarletRose::PlayerScarletRose(GObject* caster) :
	PlayerSpell(caster),
	origin(caster->getPos())
{}

void PlayerScarletRose::update()
{
	PlayerSpell::update();

	timerIncrement(timer);

	if (timer >= fireInterval && launchCount < fireCount) {
		for_irange(i, 0, 8) {
			SpaceFloat t = float_pi / FlanPolarBullet::B * i;
			gobject_ref ref = caster->space->createObject<FlanPolarBullet>(
				origin,
				0.0,
				getCasterAs<Agent>(),
				t
			);
			bullets.insert(ref);
		}

		timer -= fireInterval;
		++launchCount;
	}
}

void PlayerScarletRose::end()
{
	PlayerSpell::end();

	for (auto ref : bullets) {
		if (ref.isValid())
			caster->space->removeObject(ref);
	}
}

const string PlayerWhirlShot::name = "PlayerWhirlShot";
const string PlayerWhirlShot::description = "";
const string PlayerWhirlShot::icon = "sprites/ui/whirl_shot.png";
const float PlayerWhirlShot::cost = 10.0f;
const SpellCostType PlayerWhirlShot::costType = enum_bitwise_or(SpellCostType, stamina, initial);

const int PlayerWhirlShot::bulletCount = 8;

PlayerWhirlShot::PlayerWhirlShot(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerWhirlShot::init()
{
	SpaceFloat angleStep = float_pi * 2.0 / bulletCount;
	Agent* agent = dynamic_cast<Agent*>(caster);

	for_irange(i, 0, bulletCount)
	{
		SpaceFloat angle = i * angleStep;
		agent->bulletCheckSpawn<FlandrePolarMotionOrb>(
			agent->getPos() + SpaceVect::ray(1.0, angle),
			angle
		);
	}
}

const string PlayerDarkMist::name = "PlayerDarkMist";
const string PlayerDarkMist::description = "";
const float PlayerDarkMist::cost = 7.5f;

PlayerDarkMist::PlayerDarkMist(GObject* caster) :
	PlayerSpell(caster)
{}

void PlayerDarkMist::init()
{
	PlayerSpell::init();

	caster->space->graphicsNodeAction(&Node::setOpacity, caster->spriteID, to_uchar(128));
	caster->setInvisible(true);
}

void PlayerDarkMist::end()
{
	PlayerSpell::end();

	caster->space->graphicsNodeAction(&Node::setOpacity, caster->spriteID, to_uchar(255));
	caster->setInvisible(false);
}

const string PlayerIceShield::name = "PlayerIceShield";
const string PlayerIceShield::description = "";
const string PlayerIceShield::icon = "sprites/ui/ice_shield.png";
const float PlayerIceShield::cost = 12.5f;
const SpellCostType PlayerIceShield::costType = enum_bitwise_or3(SpellCostType, mp, initial, ongoing);

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
		
		bullets[i] = caster->space->createObject<CirnoIceShieldBullet>(
			origin + pos,
			angle - float_pi / 2.0,
			dynamic_cast<Agent*>(caster)
		);
	}
}

void PlayerIceShield::update()
{
	PlayerSpell::update();

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
	PlayerSpell::end();

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			caster->space->removeObject(ref);
		}
	}
}

