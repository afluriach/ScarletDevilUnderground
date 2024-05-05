//
//  lua_magic.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "AttributeEffects.hpp"
#include "MagicEffect.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"

template<typename T, typename... Params>
local_shared_ptr<MagicEffect> createEffect(Params... params)
{
	return make_local_shared<T>(params...);
}

namespace Lua{
	
	void Inst::addMagic()
	{
		auto effects = _state.create_table();
		_state["effects"] = effects;

		auto spells = _state.create_table();
		_state["spells"] = spells;

		auto effattr = _state.new_usertype<effect_attributes>(
			"effect_attributes",
			sol::constructors<
				effect_attributes(float, float),
				effect_attributes(float,float,float,DamageType)
			>(),
			rw_prop(effect_attributes, magnitude),
			rw_prop(effect_attributes, length),
			rw_prop(effect_attributes, radius),
			rw_prop(effect_attributes, type)
		);

		auto effdesc = _state.new_usertype<MagicEffectDescriptor>(
			"MagicEffectDescriptor",
			"flags", sol::property(&MagicEffectDescriptor::getFlags),
			"typeName", sol::property(&MagicEffectDescriptor::getTypeName)
		);
		effdesc["canApply"] = &MagicEffectDescriptor::canApply;

#define _cls MagicEffect
		auto effect = _state.new_usertype<MagicEffect>(
			"MagicEffect"
			"MagicEffect",
			"target", sol::property(&MagicEffect::getTarget),
			"agent", sol::property(&MagicEffect::getAgent),
			"length", sol::property(&MagicEffect::getLength),
			"magnitude", sol::property(&MagicEffect::getMagnitude),
			"state", sol::property(&MagicEffect::getState),
			"flags", sol::property(&MagicEffect::getFlags),
			"space", sol::property(&MagicEffect::getSpace)
		);

		auto flags = _state.new_enum<effect_flags, true>(
			"effect_flags",
			{
				enum_entry(effect_flags, none),

				enum_entry(effect_flags, immediate),
				enum_entry(effect_flags, durable),

				enum_entry(effect_flags, active),
				enum_entry(effect_flags, agent),
			}
		);

		addFuncSame(effect, isImmediate);
		addFuncSame(effect, isDurable);
		addFuncSame(effect, isActive);
		addFuncSame(effect, isAgentEffect);
		addFuncSame(effect, remove);

		effect["onEnter"] = &MagicEffect::init;
		effect["update"] = &MagicEffect::update;
		effect["onExit"] = &MagicEffect::end;

		auto spellcost = _state.new_usertype<spell_cost>(
			"spell_cost",
			rw_prop(spell_cost, initial_mp),
			rw_prop(spell_cost, initial_stamina),
			rw_prop(spell_cost, ongoing_mp),
			rw_prop(spell_cost, ongoing_stamina)
		);
#define _cls spell_cost
		addFuncSame(spellcost, none);
		addFuncSame(spellcost, initialMP);
		addFuncSame(spellcost, initialStamina);
		addFuncSame(spellcost, ongoingMP);

		auto spellparams = _state.new_usertype<spell_params>(
			"spell_params",
			sol::constructors< spell_params() >(),
			rw_prop(spell_params, name),
			rw_prop(spell_params, description),
			rw_prop(spell_params, icon),
			rw_prop(spell_params, length),
			rw_prop(spell_params, cost)
		);
#define _cls spell_params

		auto spell = _state.new_usertype<Spell>(
			"Spell",
			"agent", sol::property(&Spell::getCasterAs<Agent>),
			"descriptor", sol::property(&Spell::getDescriptor),
			"name", sol::property(&Spell::getName),
			"cost", sol::property(&Spell::getCost),
			"object", sol::property(&Spell::getCasterAs<GObject>),
			"player", sol::property(&Spell::getCasterAs<Player>),
			"space", sol::property(&Spell::getSpace),
			"time", sol::property(&Spell::getTime)
		);
#define _cls Spell

		addFuncSame(spell, runUpdate);
		addFuncSame(spell, stop);

		addFuncSame(spell, getBulletAttributes);
		addFuncSame(spell, spawnBullet);
		addFuncSame(spell, launchBullet);

		addFuncSame(spell, spawnBulletRadius);
		addFuncSame(spell, bulletCircle);

		auto spell_desc = _state.new_usertype<SpellDesc>("SpellDescriptor");
#define _cls SpellDesc

		addFuncSame(spell_desc, getName);
		addFuncSame(spell_desc, getDescription);
		addFuncSame(spell_desc, getIcon);

		addFuncSame(spell_desc, getCost);
	}

}
