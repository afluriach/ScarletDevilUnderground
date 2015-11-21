//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "LuaAPI.hpp"

namespace Lua{

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }

    //Lua API functions:
    int log(lua_State* L)
    {
        //The first argument must be a string
        int nArgs = lua_gettop(L);
        
        if(nArgs < 1){
            error(L, "log: parameter required");
            return 0;
        }
        
        if(!lua_isstring(L,1)){
            error(L, "log: first parameter must be string.");
            return 0;
        }

        //paramters are ones-based, arg count is at index 0;
        cocos2d::log("%s", lua_tostring(L,1));
        
        return 0;
    }

}