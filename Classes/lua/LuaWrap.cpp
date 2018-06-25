//
//  LuaWrap.cpp
//  Koumachika
//
//  Created by Toni on 12/14/15.
//
//

#include "Prefix.h"

#include "LuaWrap.h"
#include "macros.h"

namespace Lua{

void Class::installClass(lua_State* L)
{
    makeMetatable(L);
    
    //Add all wrapped methods
    
    //Get metatable
    lua_getglobal(L, name.c_str());

    foreach(auto methodEntry, methods){
        string name = methodEntry.first;
        lua_CFunction wrapper = methodEntry.second;
        
        //Add method
        
        //for table assign, value is top of stack and key is second.
        lua_pushstring(L, name.c_str());
        lua_pushcfunction(L, wrapper);
        
        lua_rawset(L, -3);
    }
}

void Class::makeMetatable(lua_State* L)
{
    //check if it already exists, which shouldn't happen unless the
    //create constructor was inappopriately invoked.

    lua_newtable(L);
    lua_setglobal(L,name.c_str());
    
    //Set __index
    lua_getglobal(L,name.c_str());
    lua_pushstring(L, "__index");
    lua_getglobal(L,name.c_str());
    lua_rawset(L, -3);
}

//statics

void Class::setMetatable(const string& name, lua_State* L)
{
    if(!lua_istable(L, -1))
        throw runtime_error("setMetatable: top of stack must table.");
    
    lua_getglobal(L, name.c_str());
    
    if(lua_isnil(L, -1))
        throw runtime_error(name + " metatable not found.");
    
    lua_setmetatable(L, -2);
}

void Class::installClasses(lua_State* L)
{
    if(!init){
        makeClasses();
        init = true;
    }

    foreach(auto classEntry, classes){
        classEntry.second.installClass(L);
    }
}


}
