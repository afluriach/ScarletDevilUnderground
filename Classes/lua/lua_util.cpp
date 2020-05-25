//
//  lua_util.cpp
//  Koumachika
//
//  Created by Toni on 2/29/20.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"
#include "SpellUtil.hpp"
#include "value_map.hpp"

namespace Lua{    

void Inst::addUtil()
{
	auto util = _state.create_table();
	_state["util"] = util;

	//Lua does not support passing primitves by reference, thus sol does not
	//support wrapping the C++ version of these functions.
	util["timerDecrement"] = [](SpaceFloat input) -> SpaceFloat {
		SpaceFloat result = input;
		timerDecrement(result);
		return result;
	};
	util["timerIncrement"] = [](SpaceFloat input) -> SpaceFloat {
		SpaceFloat result = input;
		timerIncrement(result);
		return result;
	};

	util["isinf"] = &isinf<double>;
	util["isnan"] = &isnan;

	cFuncSame(util, getStringOrDefault);
	cFuncSame(util, getIntOrDefault);
	cFuncSame(util, getFloatOrDefault);
	cFuncSame(util, getBoolOrDefault);

	cFuncSame(util, explosion);
	cFuncSame(util, radialDamageArea);
}
}
