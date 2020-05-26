//
//  PlayerSpell.cpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#include "Prefix.h"

#include "graphics_context.hpp"
#include "MagicEffect.hpp"
#include "Player.hpp"
#include "PlayerSpell.hpp"
#include "SpellDescriptor.hpp"

const string PlayerBatMode::name = "PlayerBatMode";
const string PlayerBatMode::description = "";
const string PlayerBatMode::icon = "sprites/ui/bat_mode.png";
const spell_cost PlayerBatMode::cost = spell_cost::ongoingMP(5.0f);

PlayerBatMode::PlayerBatMode(GObject* caster, const SpellDesc* desc, unsigned int id) :
	ApplySelfEffect(caster, desc, id, spell_params{ -1.0, -1.0, PlayerBatMode::cost }, app::getEffect("BatTransform"))
{}

PlayerBatMode::~PlayerBatMode()
{
}

const string LavaeteinnSpell::name = "LavaeteinnSpell";
const string LavaeteinnSpell::description = "";
const string LavaeteinnSpell::icon = "sprites/ui/lavaeteinn.png";
const spell_cost LavaeteinnSpell::cost = spell_cost::initialStamina(12.5f);

const SpaceFloat LavaeteinnSpell::length = 2.0 / 3.0;
const SpaceFloat LavaeteinnSpell::angleWidth = float_pi / 2.0;
const SpaceFloat LavaeteinnSpell::angular_speed = angleWidth * 2.0 / length;
const int LavaeteinnSpell::bulletSpawnCount = 8;

LavaeteinnSpell::LavaeteinnSpell(GObject* caster, const SpellDesc* desc, unsigned int id) :
	MeleeAttack(
		caster,
		desc,
		id,
		spell_params{ length, 0.0, cost },
		melee_params{
			length,
			1.5,
			angleWidth,
			app::getBullet("lavaeteinn"),
			app::getBullet("flandreFastOrb1"),
			2.0,
			8
		}
	)
{}

LavaeteinnSpell::~LavaeteinnSpell()
{
}

const string PlayerCounterClock::name = "PlayerCounterClock";
const string PlayerCounterClock::description = "";
const string PlayerCounterClock::icon = "sprites/ui/counterclock.png";
const spell_cost PlayerCounterClock::cost = spell_cost::ongoingMP(7.5f);

const SpaceFloat PlayerCounterClock::offset = 0.75;
const SpaceFloat PlayerCounterClock::angular_speed = 9.0;

PlayerCounterClock::PlayerCounterClock(GObject* caster, const SpellDesc* desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ -1.0, 0.0, PlayerCounterClock::cost })
{}

PlayerCounterClock::~PlayerCounterClock()
{
}

void PlayerCounterClock::init()
{
	Player* p = getCasterAs<Player>();
	SpaceVect pos = caster->getPos();
	auto props = app::getBullet("flandreCounterClockBullet");

	if (p) {
		p->increment(Attribute::inhibitFiring);
	}

	bullets = spawnBulletRadius(props, 2.0 + offset, 4);
}

void PlayerCounterClock::update()
{
	angular_pos += angular_speed * app::params.secondsPerFrame;
	bulletCircle(bullets, 2.0 + offset, angular_pos);
}

void PlayerCounterClock::end()
{
	Player* p = getCasterAs<Player>();

	if (p) {
		p->decrement(Attribute::inhibitFiring);
	}

	for (auto ref : bullets)
	{
		if (ref.isValid()) {
			getSpace()->removeObject(ref);
		}
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

PlayerIceShield::PlayerIceShield(GObject* caster, const SpellDesc* desc, unsigned int id) :
	Spell(caster, desc, id, spell_params{ -1.0, 0.0, PlayerIceShield::cost })
{
	props = app::getBullet("cirnoIceShieldBullet");
}

PlayerIceShield::~PlayerIceShield()
{
}

void PlayerIceShield::init()
{
	SpaceVect origin = caster->getPos();

	bullets = spawnBulletRadius(props, distance, bulletCount);
}

void PlayerIceShield::update()
{
	//Update angle based on linear speed
	SpaceFloat dp = app::params.secondsPerFrame * speed;
	crntAngle += dp * inv_circumference * float_pi * 2.0;

	if (crntAngle >= 2.0*float_pi) {
		crntAngle -= 2.0*float_pi;
	}

	bulletCircle(bullets, distance, crntAngle);
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

