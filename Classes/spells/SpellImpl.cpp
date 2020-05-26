//
//  SpellImpl.cpp
//  Koumachika
//
//  Created by Toni on 2/23/20.
//
//

#include "Prefix.h"

#include "Bullet.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "MagicEffectSystem.hpp"
#include "SpellDescriptor.hpp"
#include "SpellImpl.hpp"

spell_params ScriptedSpell::getParams(string clsName)
{
	sol::table cls = GSpace::scriptVM->_state["spells"][clsName];

	if (!cls) {
		log("ScriptedSpell class %s does not exist!", clsName);
		return spell_params();
	}

	sol::function f = cls["getParams"];
	return f ? f() : spell_params();
}

ScriptedSpell::ScriptedSpell(GObject* caster, const SpellDesc* desc, unsigned int id, string clsName) :
	Spell(caster,desc,id, getParams(clsName)),
	clsName(clsName)
{
	auto cls = GSpace::scriptVM->_state["spells"][clsName];
	Spell* super_this = this;

	if (cls) {
		obj = cls(super_this, caster);
	}
}

ScriptedSpell::~ScriptedSpell()
{
}

void ScriptedSpell::init()
{
	if (obj) {
		sol::function f = obj["onEnter"];
		if (f) f(obj);
	}
}

void ScriptedSpell::update()
{
	if (obj) {
		sol::function f = obj["update"];
		if (f) f(obj);
	}
}

void ScriptedSpell::end()
{
	if (obj) {
		sol::function f = obj["onExit"];
		if (f) f(obj);
	}
}

void ScriptedSpell::onBulletRemove(Bullet* b)
{
	if (obj) {
		sol::function f = obj["onBulletRemove"];
		if (f) f(obj, b);
	}
}

ApplySelfEffect::ApplySelfEffect(
	GObject* caster,
	const SpellDesc* desc,
	unsigned int id,
	spell_params params,
	const MagicEffectDescriptor* effect
) :
	Spell(caster, desc, id, params),
	effect(effect)
{
}

ApplySelfEffect::~ApplySelfEffect()
{
}

void ApplySelfEffect::init()
{
	effectID = caster->applyMagicEffect(effect, effect_attributes(0.0f, -1.0f));
}

void ApplySelfEffect::end()
{
	if (effectID != 0) {
		getSpace()->magicEffectSystem->removeEffect(effectID);
	}
}

MeleeAttack::MeleeAttack(
	GObject* caster,
	const SpellDesc* desc,
	unsigned int id,
	spell_params params,
	melee_params melee
) :
	Spell(caster, desc, id, params),
	melee(melee)
{
}

void MeleeAttack::init()
{
	SpaceFloat angle = canonicalAngle(caster->getAngle() - melee.sideAngleWidth);
	angularPos = angle;
	fireTimer = (melee.fireCount > 0) ? length / melee.fireCount : length;
	angular_speed = melee.sideAngleWidth * 2.0 / melee.length;

	bullet = spawnBullet(
		melee.melee,
		SpaceVect::ray(melee.swingDist, angle),
		SpaceVect::zero,
		angle,
		angular_speed
	);
}

void MeleeAttack::update()
{
	timerDecrement(fireTimer);
	timerIncrement(angularPos, angular_speed);

	if (bullet.isValid()) {
		bullet.get()->setPos(caster->getPos() + SpaceVect::ray(melee.swingDist, angularPos));
	}

	if (melee.bullet && melee.fireCount > 0 && fireTimer <= 0.0) {
		launchBullet(melee.bullet, SpaceVect::ray(melee.launchDist, angularPos), angularPos);
		fireTimer = length / melee.fireCount;
	}
}

void MeleeAttack::end()
{
	if (bullet.isValid()) {
		getSpace()->removeObject(bullet);
	}
}
