//
//  LuaConvert.cpp
//  FlansBasement
//
//  Created by Toni on 12/5/15.
//
//

#include "Prefix.h"

class Spellcaster;

namespace Lua{

Vec2 getVec2FromTable(LuaRef t)
{
    if(t.isNil()){
        log("getVec2FromTable: nil table vector");
        return Vec2(0,0);
    }
    
    if(t["x"].isNumber() && t["y"].isNumber()){
        return Vec2(getFloat(t["x"]), getFloat(t["y"]));
    }
    if(t[1].isNumber() && t[2].isNumber()){
        return Vec2(getFloat(t[1]), getFloat(t[2]));
    }
    
    log("getVec2FromTable: not a vector");
    return Vec2(0,0);
}

map<string,string> getStringMapFromTable(LuaRef table, lua_State* state)
{
    table.push(state);
    lua_pushnil(state);
    
    map<string,string> result;
    
    while(lua_next(state, -2))
    {
        //lua_next puts value on top of stack above key.

        //Copy key before consuming it.
        lua_pushvalue(state, -2);
        
        string key(lua_tostring(state, -1));
        string value(lua_tostring(state, -2));
        
        log("%s,%s", key.c_str(), value.c_str());
        result[key] = value;
        
        lua_pop(state,2);
    }
    //Pop the table;
    lua_pop(state,1);
    
    return result;
}

function<void()> makeFunctorFromLuaFunction(LuaRef ref)
{
    return [=]() -> void{
        ref();
    };
}

list<LuaRef> getArgs(lua_State* L)
{
    int nArgs = lua_gettop(L);
    list<LuaRef> result;
    
    for(int i=0;i<nArgs; ++i){
        LuaRef ref(L);
        ref.pop(L);

        //For some reason this first log statement always prints the first parameter.
        //log("arg %d: %s", i+1, ref.tostring().c_str());
        result.push_back(ref);
    }
    result.reverse();

    //But here it shows them stored correctly.
//    foreach(LuaRef ref, result){
//        log("arg %s", ref.tostring().c_str());
//    }
    
    return result;
}

float getFloat(LuaRef r)
{
    return r.cast<float>();
}

#define check_type(type) \
if(typeStr == #type){ \
    result = LuaRef(L, dynamic_cast<type*>(o)); \
}

//Convert userdata pointer to a derived class if applicable,
//otherwise return nil
LuaRef convertObjectUserdata(GObject* o, const string& typeStr, lua_State* L)
{
    LuaRef result(L);

    check_type(Spellcaster)
    
    return result;
}

void check_integer_value(LuaRef ref)
{
    double d = ref.cast<double>();
    if(floor(d) != d)
        throw lua_type_error("Attempt to cast non-integer Lua number to integer.");
}

}