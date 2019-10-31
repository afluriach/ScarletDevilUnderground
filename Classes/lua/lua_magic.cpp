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
#define _cls MagicEffect
		auto effect = _state.new_usertype<MagicEffect>(
			"MagicEffect"
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
