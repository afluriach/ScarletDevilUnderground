//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "LuaAPI.hpp"

using namespace std;

namespace Lua{

    shared_ptr<Data> Data::copy(lua_State* L)
    {
        if(lua_isnumber(L, -1)){
            double n = lua_tonumber(L, -1);
            return make_shared<Number>(n);
        }
        if(lua_isstring(L, -1)){
            const char* c = lua_tostring(L, -1);
            return make_shared<String>(c);
        }
    }

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