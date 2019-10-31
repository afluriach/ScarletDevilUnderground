//
//  lua_magic.cpp
//  Koumachika
//
//  Created by Toni on 10/31/19.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "GSpace.hpp"
#include "MagicEffect.hpp"
#include "LuaAPI.hpp"

namespace Lua{
    
	void Inst::addMagic()
	{
		auto effects = _state.create_table();
		_state["effects"] = effects;

#define _cls MagicEffect
		auto effect = _state.new_usertype<MagicEffect>(
			"MagicEffect"
		);

		_state.new_enum<MagicEffect::flags, true>(
			"MagicEffect_flags",
			{
				enum_entry(MagicEffect::flags, immediate),
				enum_entry(MagicEffect::flags, indefinite),
				enum_entry(MagicEffect::flags, timed),

				enum_entry(MagicEffect::flags, active),
			}
		);

		effect["make_flags_bitfield"] = sol::overload(
			&make_enum_bitfield<MagicEffect::flags>,
			[](MagicEffect::flags a, MagicEffect::flags b) -> MagicEffect::flag_bits {
				return make_enum_bitfield(a) | make_enum_bitfield(b);
			}
		);

		addFuncSame(effect, getSpace);
		addFuncSame(effect, isImmediate);
		addFuncSame(effect, isTimed);
		addFuncSame(effect, isActive);

		effect["onEnter"] = &MagicEffect::init;
		effect["update"] = &MagicEffect::update;
		effect["onExit"] = &MagicEffect::end;

		addFuncSame(effect, agent);
		addFuncSame(effect, length);
		addFuncSame(effect, magnitude);
		addFuncSame(effect, crntState);
		addFuncSame(effect, _flags);

	}
}
