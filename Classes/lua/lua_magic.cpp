//
//  lua_magic.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AttributeEffects.hpp"
#include "MiscMagicEffects.hpp"
#include "MagicEffect.hpp"
#include "LuaAPI.hpp"
#include "Player.hpp"
#include "Spell.hpp"
#include "SpellDescriptor.hpp"
#include "spell_types.hpp"

template<typename T, typename... Params>
shared_ptr<MagicEffect> createEffect(Params... params)
{
	return make_shared<T>(params...);
}

namespace Lua{
    
	void Inst::addMagic()
	{
		auto effects = _state.create_table();
		_state["effects"] = effects;

		auto spells = _state.create_table();
		_state["spells"] = spells;

#define _cls MagicEffect
		auto effect = _state.new_usertype<MagicEffect>(
			"MagicEffect"
		);

		auto flags = _state.new_enum<effect_flags, true>(
			"effect_flags",
			{
				enum_entry(effect_flags, none),

				enum_entry(effect_flags, immediate),
				enum_entry(effect_flags, indefinite),
				enum_entry(effect_flags, timed),

				enum_entry(effect_flags, active),
			}
		);

		addFuncSame(effect, getSpace);
		addFuncSame(effect, isImmediate);
		addFuncSame(effect, isTimed);
		addFuncSame(effect, isActive);
		addFuncSame(effect, remove);

		effect["onEnter"] = &MagicEffect::init;
		effect["update"] = &MagicEffect::update;
		effect["onExit"] = &MagicEffect::end;

		addFuncSame(effect, target);
		addFuncSame(effect, length);
		addFuncSame(effect, magnitude);
		addFuncSame(effect, crntState);
		addFuncSame(effect, _flags);

		auto spellcost = _state.new_usertype<spell_cost>("spell_cost");
#define _cls spell_cost
		addFuncSame(spellcost, initial_mp);
		addFuncSame(spellcost, initial_stamina);
		addFuncSame(spellcost, ongoing_mp);
		addFuncSame(spellcost, ongoing_stamina);

		addFuncSame(spellcost, none);
		addFuncSame(spellcost, initialMP);
		addFuncSame(spellcost, initialStamina);
		addFuncSame(spellcost, ongoingMP);

		auto spellparams = _state.new_usertype<spell_params>(
			"spell_params",
			sol::constructors<
				spell_params(),
				spell_params(SpaceFloat),
				spell_params(SpaceFloat, SpaceFloat), 
				spell_params(SpaceFloat,SpaceFloat,spell_cost)
			>()
		);
#define _cls spell_params

		auto spell = _state.new_usertype<Spell>("Spell");
#define _cls Spell

		spell["onEnter"] = &Spell::init;
		spell["update"] = &Spell::update;
		spell["onExit"] = &Spell::end;

		spell["getCasterObject"] = &Spell::getCasterAs<GObject>;
		spell["getCasterAsAgent"] = &Spell::getCasterAs<Agent>;
		spell["getCasterAsPlayer"] = &Spell::getCasterAs<Player>;
		spell["getSpace"] = &Spell::getSpace;

		auto spell_desc = _state.new_usertype<SpellDesc>("SpellDescriptor");
#define _cls SpellDesc

		addFuncSame(spell_desc, getName);
		addFuncSame(spell_desc, getDescription);
		addFuncSame(spell_desc, getIcon);

		addFuncSame(spell_desc, getCost);

		addFuncSame(spell_desc, generate);
		addFuncSame(spell_desc, getGenerator);
	}

}
