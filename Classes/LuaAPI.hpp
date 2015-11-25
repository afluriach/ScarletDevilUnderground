//
//  Lua.hpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "cocos2d.h"
#include "lua.hpp"
#include "LuaBridge.h"

#include "util.h"

using namespace std;
using namespace luabridge;

namespace Lua
{
    static const string lauArgTag = "luaScript";

    //Raise Lua exception
    void error(lua_State* L, const string& msg);
    
    map<string,string> getStringMapFromTable(LuaRef table);

    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        Inst();
        ~Inst();
        
        void installApi();
        void installFunction(lua_CFunction func, const string& name);
        void runString(const string& str);
        void runFile(const string& path);
        vector<LuaRef> call(const string& name, const vector<LuaRef>& params);
        
        //Helper for making Lua data, since LuaRef requires the lua state.
        template<typename T>
        LuaRef makeRef(T data)
        {
            return LuaRef(state,data);
        }
    private:
        lua_State *state;
    };
}

#endif /* Lua_hpp */
