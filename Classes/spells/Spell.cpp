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

Spell::Spell(GObject* caster, spell_params params) :
	caster(caster),
	length(params.length),
	updateInterval(params.updateInterval),
	_cost(params.cost)
{}

Spell::~Spell() {}

bool Spell::isActive() const {
	return crntState == state::active;
}

GSpace* Spell::getSpace() const {
	return caster->space;
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
		runEnd();
	}
}

void Spell::runEnd()
{
	crntState = state::ending;
	end();
	caster->crntSpell.reset();
	crntState = state::expired;
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

SpellGeneratorType ScriptedSpell::generator(string clsName)
{
	return [clsName](GObject* caster) -> shared_ptr<Spell> {
		return make_shared<ScriptedSpell>(caster, clsName);
	};
}

ScriptedSpell::ScriptedSpell(GObject* caster, string clsName) :
	Spell(caster, getParams(clsName)),
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

shared_ptr<SpellDesc> ScriptedSpell::getDescriptor()
{
	return make_shared<ScriptedSpellDescriptor>(clsName);
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

ApplySelfEffect::ApplySelfEffect(GObject* caster,  spell_params params, shared_ptr<MagicEffectDescriptor> effect) :
	Spell(caster, params),
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
