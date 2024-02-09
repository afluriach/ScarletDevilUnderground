//
//  MagicEffect.cpp
//  Koumachika
//
//  Created by Toni on 11/28/18.
//
//

#include "Prefix.h"

#include "AIUtil.hpp"
#include "LuaAPI.hpp"
#include "MagicEffect.hpp"
#include "MagicEffectSystem.hpp"

MagicEffect::MagicEffect(effect_params params) :
target(params.target),
agent(dynamic_cast<Agent*>(params.target)),
length(params.attr.length),
magnitude(params.attr.magnitude),
_flags(params.flags),
id(params.id),
desc(params.desc),
damageType(params.attr.type),
crntState(state::created)
{
	if (_flags == effect_flags::none) {
		log0("Warning, empty MagicEffect created.");
	}
}

GSpace* MagicEffect::getSpace() const {
	return target->space;
}

bool MagicEffect::isImmediate() const
{
	return bitwise_and_bool(_flags, effect_flags::immediate);
}

bool MagicEffect::isDurable() const
{
	return bitwise_and_bool(_flags, effect_flags::durable);
}

bool MagicEffect::isActive() const
{
	return bitwise_and_bool(_flags, effect_flags::active);
}

bool MagicEffect::isAgentEffect() const
{
	return bitwise_and_bool(_flags, effect_flags::agent);
}

void MagicEffect::remove()
{
	getSpace()->magicEffectSystem->removeEffect(id);
}

effect_flags ScriptedMagicEffect::getFlags(string clsName)
{
	sol::table cls = GSpace::scriptVM->_state["effects"][clsName];

    if (!cls.valid()) {
		log1("%s does not exist", clsName);
		return effect_flags::none;
	}

	sol::object obj = cls["flags"];

	return obj ? obj.as<effect_flags>() : effect_flags::none;
}

ScriptedMagicEffect::ScriptedMagicEffect(effect_params params, string clsName) :
	MagicEffect(params),
	clsName(clsName)
{
	auto cls = GSpace::scriptVM->_state["effects"][clsName];
	MagicEffect* super_this = this;

    if (!cls.valid()) {
		log1("%s not found", clsName);
	}
	else {
		obj = cls(super_this);
	}
}

void ScriptedMagicEffect::init()
{
	if (obj) {
		sol::function f = obj["onEnter"];
		if (f) f(obj);
	}
}

void ScriptedMagicEffect::update()
{
	if (obj) {
		sol::function f = obj["update"];
		if (f) f(obj);
	}
}

void ScriptedMagicEffect::end()
{
	if (obj) {
		sol::function f = obj["onExit"];
		if (f) f(obj);
	}
}
