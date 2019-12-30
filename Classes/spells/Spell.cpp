//
//  Spell.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "MagicEffectSystem.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "SpellSystem.hpp"

spell_cost spell_cost::none()
{
	return spell_cost{ 0.0f,0.0f,0.0f,0.0f };
}

spell_cost spell_cost::initialMP(float mp)
{
	return spell_cost{ mp, 0.0f, 0.0f, 0.0f };
}

spell_cost spell_cost::initialStamina(float stamina)
{
	return spell_cost{ 0.0f, stamina, 0.0f, 0.0f };
}

spell_cost spell_cost::ongoingMP(float mp)
{
	return spell_cost{ 0.0f, 0.0f, mp, 0.0f };
}

spell_params::spell_params(
	SpaceFloat length,
	SpaceFloat updateInterval,
	spell_cost cost
) :
	length(length),
	updateInterval(updateInterval),
	cost(cost)
{
}

Spell::Spell(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, spell_params params) :
	caster(caster),
	descriptor(desc),
	id(id),
	length(params.length),
	updateInterval(params.updateInterval),
	_cost(params.cost)
{}

Spell::~Spell() {}

GSpace* Spell::getSpace() const {
	return caster->space;
}

bullet_attributes Spell::getBulletAttributes(shared_ptr<bullet_properties> props) const
{
	return caster->getBulletAttributes(props);
}

gobject_ref Spell::spawnBullet(
	shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceVect velocity,
	SpaceFloat angle,
	SpaceFloat angularVelocity
) {
	return caster->spawnBullet(props, displacement, velocity, angle, angularVelocity);
}

gobject_ref Spell::launchBullet(
	shared_ptr<bullet_properties> props,
	SpaceVect displacement,
	SpaceFloat angle,
	SpaceFloat angularVelocity,
	bool obstacleCheck
) {
	return caster->launchBullet(props, displacement, angle, angularVelocity, obstacleCheck);
}

void Spell::runUpdate()
{
	timerIncrement(t);
	timerIncrement(lastUpdate);

	if (updateInterval >= 0.0 && lastUpdate >= updateInterval) {
		update();
		lastUpdate -= updateInterval;
	}

	if (length > 0.0 && t >= length) {
		stop();
	}
}

void Spell::stop()
{
	getSpace()->spellSystem->stopSpell(id);
}

shared_ptr<SpellDesc> Spell::getDescriptorByName(const string& name)
{
	auto it = spellDescriptors.find(name);

	if (it != spellDescriptors.end())
		return it->second;
	else
		return nullptr;
}

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

ScriptedSpell::ScriptedSpell(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, string clsName) :
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

ApplySelfEffect::ApplySelfEffect(GObject* caster, shared_ptr<SpellDesc> desc, unsigned int id, spell_params params, shared_ptr<MagicEffectDescriptor> effect) :
	Spell(caster, desc, id, params),
	effect(effect)
{
}

void ApplySelfEffect::init()
{
	effectID = caster->applyMagicEffect(effect, 0.0f, -1.0f);
}

void ApplySelfEffect::end()
{
	if (effectID != 0) {
		getSpace()->magicEffectSystem->removeEffect(effectID);
	}
}
