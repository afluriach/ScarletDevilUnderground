//
//  LuaConvert.cpp
//  Koumachika
//
//  Created by Toni on 12/5/15.
//
//

#include "Prefix.h"

#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "LuaWrap.h"
#include "macros.h"
#include "scenes.h"

class Spellcaster;

namespace Lua{

bool tableIsVec2(LuaRef t)
{
    if(t.isNil()){
        return false;
    }
    
    return (t["x"].isNumber() && t["y"].isNumber()) ||
           (t[1].isNumber() && t[2].isNumber());
}

IntVec2 getInt2FromTable(LuaRef t)
{
    if(!tableIsVec2(t)){
        log("getInt2FromTable: not a table vector");
        return IntVec2(0,0);
    }
    
    if(t["x"].isNumber() && t["y"].isNumber()){
        return IntVec2(t["x"].cast<int>(), t["y"].cast<int>());
    }
    if(t[1].isNumber() && t[2].isNumber()){
        return IntVec2(t[1].cast<int>(), t[2].cast<int>());
    }
    
    //Should be unreachable, since it must have passed the tableIsVec2 test.
    log("getVec2FromTable: not a vector");
    return IntVec2(0,0);
}


Vec2 getVec2FromTable(LuaRef t)
{
    if(!tableIsVec2(t)){
        log("getVec2FromTable: not a table vector");
        return Vec2(0,0);
    }
    
    if(t["x"].isNumber() && t["y"].isNumber()){
        return Vec2(getFloat(t["x"]), getFloat(t["y"]));
    }
    if(t[1].isNumber() && t[2].isNumber()){
        return Vec2(getFloat(t[1]), getFloat(t[2]));
    }
    
    //Should be unreachable, since it must have passed the tableIsVec2 test.
    log("getVec2FromTable: not a vector");
    return Vec2(0,0);
}

unordered_map<string,string> getStringMapFromTable(LuaRef table, lua_State* state)
{
    table.push(state);
    lua_pushnil(state);
    
    unordered_map<string,string> result;
    
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
    
    for_irange(i,0,nArgs){
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

void check_integer_value(LuaRef ref)
{
    double d = ref.cast<double>();
    if(floor(d) != d)
        throw lua_type_error("Attempt to cast non-integer Lua number to integer.");
}

//The ID field cannot be wrapped, as it is used to get the pointer to the actual object.
GObject* convert<GObject*>::convertFromLua(const string& name, int argNum, LuaRef ref)
{
    unsigned int uuid = ref["uuid"].cast<unsigned int>();
    return GScene::getSpace()->getObject(uuid);
}
LuaRef convert<GObject*>::convertToLua(GObject* obj, lua_State* L)
{
    lua_newtable(L);
    lua_pushstring(L, "uuid");
    lua_pushnumber(L, obj->uuid);
    lua_settable(L, -3);
    
    Class::setMetatable("GObject", L);
    
    LuaRef ref(L);
    ref.pop(L);
    return ref;
}

//Check if the data is a name or an object
[[deprecated("Does not use Class mapping")]]
GObject* getObjectFromLuaData(LuaRef ref)
{
    GSpace* space = GScene::getSpace();
    if(ref.isNumber())
        return space->getObject(ref.cast<unsigned int>());
    else if(ref.isString())
        return space->getObject(ref.tostring());
    else{
        log("getObjectFromLuaData: input is not a name or an ID.");
        return nullptr;
    }
}

ValueMap convert<ValueMap>::convertFromLua(const string& name, int argNum, LuaRef ref)
{
    ValueMap vm;
    
    for(auto it = Iterator(ref); !it.isNil(); ++it)
    {
        string key = it.key().tostring();
        LuaRef lval = it.value();
        
        if(lval.isNumber())
            vm[key] = Value(lval.cast<double>());
        else if(lval.isString())
            vm[key] = Value(lval.tostring());
        //The only compound data type currently expected is a Vector2.
        else if(lval.isTable() && Lua::tableIsVec2(lval))
        {
            //Expand to key_[x,y] keys.
            Vec2 v = Lua::getVec2FromTable(lval);
            vm[key + "_x"] = Value(v.x);
            vm[key + "_y"] = Value(v.y);
        }
    }
    
    return vm;
}
LuaRef convert<ValueMap>::convertToLua(ValueMap map, lua_State* L)
{
    LuaRef table = newTable(L);
    
    foreach(auto entry, map)
    {
        if(entry.second.isNumber()){
            table[entry.first] = entry.second.asDouble();
        }
        else if(entry.second.isString()){
            table[entry.first] = entry.second.asString();
        }
    }
    
    return table;
}


}
