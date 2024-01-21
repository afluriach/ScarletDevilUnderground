//
//  sol_util.cpp
//  Koumachika
//
//  Created by Toni on 3/11/21.
//
//

#include "Prefix.h"

#include "sol_util.hpp"

namespace sol {

bool hasMethod(sol::table obj, const string& name)
{
	sol::function f = obj[name];
	return to_bool(f);
}

void printErrorMessage(lua_State* state)
{
    print("    %s", lua_tostring(state, -1));
}

}
