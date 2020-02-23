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

ScriptedSpell::ScriptedSpell(GObject* caster, local_shared_ptr<SpellDesc> desc, unsigned int id, string clsName) :
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

ApplySelfEffect::ApplySelfEffect(GObject* caster, local_shared_ptr<SpellDesc> desc, unsigned int id, spell_params params, local_shared_ptr<MagicEffectDescriptor> effect) :
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
