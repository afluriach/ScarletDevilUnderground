//
//  lua_util.cpp
//  Koumachika
//
//  Created by Toni on 2/29/20.
//
//

#include "Prefix.h"

#include "Graphics.h"
#include "LuaAPI.hpp"
#include "SpellUtil.hpp"

namespace Lua{	

void Inst::addUtil()
{
	auto util = _state.create_table();
	_state["util"] = util;

	cFuncSame(util, getIntSuffix);
	cFuncSame(util, splitString);

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
 
	cFuncSame(util, canonicalAngle);
	cFuncSame(util, invertDirection);

	util["isinf"] = &std::isinf<double>;
#ifdef _WIN32
	util["isnan"] = &isnan;
#else
	util["isnan"] = &std::isnan<double>;
#endif

	cFuncSame(util, getPoints);
	cFuncSame(util, getAdjacentTiles);

	cFuncSame(util, explosion);
	cFuncSame(util, radialDamageArea);
  
	cFuncSame(util, toCocosAngle);

	util["toCocos"] = static_cast<Vec2(*)(const SpaceVect&)>(&toCocos);
	util["toColor3B"] = sol::overload(
		static_cast<Color3B(*)(const Color4F&)>(&toColor3B),
		static_cast<Color3B(*)(const string&)>(&toColor3B)
	);
}
}
