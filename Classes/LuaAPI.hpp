//
//  Lua.hpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

#include <vector>

#include "cocos2d.h"
#include "lua.hpp"
#include "util.h"

using namespace std;

namespace Lua
{
    class Data
    {
    };
    
    //Raise Lua exception
    void error(lua_State* L, const string& msg);
    
    //Lua API functions:
    int log(lua_State* L);


    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        inline Inst()
        {
            state = luaL_newstate();
            //Load standard libraries
            luaL_openlibs(state);
            installApi();
        }
        inline ~Inst()
        {
            lua_close(state);
        }
        
        inline void installApi()
        {
            installFunction(log,"log");
        }
        
        inline void installFunction(lua_CFunction func, const string& name)
        {
            lua_pushcfunction(state, func);
            lua_setglobal(state, name.c_str());
        }
        
        inline void runString(const string& str)
        {        
            int error = luaL_dostring(state, str.c_str());
            
            if(error)
                cocos2d::log("runString Lua error.");
        }
        
        inline void runFile(const string& path)
        {
            runString(loadTextFile(path));
        }
    
        inline vector<Data> call(const vector<Data>& params)
        {
        };
    private:
        lua_State *state;
    };
}

#endif /* Lua_hpp */
