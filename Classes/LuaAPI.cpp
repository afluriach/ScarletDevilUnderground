//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"

namespace Lua{

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }

//Lua API methods

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

    Inst::Inst()
    {
        state = luaL_newstate();
        //Load standard libraries
        luaL_openlibs(state);
        installApi();
    }
    Inst::~Inst()
    {
        lua_close(state);
    }
    
    void Inst::installFunction(lua_CFunction func, const string& name)
    {
        lua_pushcfunction(state, func);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::runString(const string& str)
    {        
        int error = luaL_dostring(state, str.c_str());
        
        if(error)
            log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
    {
        runString(loadTextFile(path));
    }
    
    void Inst::runObjectFile(const string& path)
    {
        luaL_loadfile(state, getRealPath(path).c_str());
        lua_pcall(state, 0, LUA_MULTRET, 0);
    
        runString(loadTextFile(path));
    }

     vector<LuaRef> Inst::call(const string& name, const vector<LuaRef>& params)
    {
        int top = lua_gettop(state);
        
        lua_pushglobaltable(state);
        lua_getfield(state, -1, name.c_str());
        //Remove global table after pushing function to call
        lua_remove(state, -2);
        
        foreach(LuaRef r, params)
        {
            r.push(state);
        }
        
        lua_call(state, params.size(), LUA_MULTRET);
        
        int nResults = lua_gettop(state) - top;
        vector<LuaRef> results;
        
        for(int i=0;i<nResults; ++i){
            LuaRef ref(state);
            ref.pop(state);
            results.push_back(ref);
        }
        reverse(results.begin(), results.end());
        return results;
    };
    
//Lua data access helpers
    #define requireNonNil(ref, L, msg) \
        if(ref.isNil()){ \
            error(L, msg); \
            return 0; \
        }

    float getFloat(LuaRef r)
    {
        return r.cast<float>();
    }
    
    Vec2 getVec2FromTable(LuaRef t)
    {
        if(t.isNil()){
            log("getVec2FromTable: nil table vector");
            return Vec2(0,0);
        }
        
        if(t["x"].isNumber() && t["y"].isNumber()){
            return Vec2(getFloat(t["x"]), getFloat(t["x"]));
        }
        if(t[1].isNumber() && t[2].isNumber()){
            return Vec2(getFloat(t[1]), getFloat(t[2]));
        }
        
        log("getVec2FromTable: not a vector");
        return Vec2(0,0);
    }

    //Lua API functions:
    int luaLog(lua_State* L)
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
        log("%s", lua_tostring(L,1));
        
        return 0;
    }
    
    int createObject(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        GSpace* space = GScene::getSpace();
        if(!space){
            error(L, "createObject: cannot create object in this scene.");
            return 0;
        }
        
        if(nArgs != 1){
            error(L, "createObject: single table params required");
            return 0;
        }
        
        LuaRef arg(L);
        arg.pop(L);
        
        if(!arg.isTable()){
            error(L, "createObject: single table params required");
            return 0;
        }
        
        //Position and dimension are x,y vector
        LuaRef pos = arg["pos"];
        LuaRef props = arg["props"];
        
        LuaRef name = arg["name"];
        LuaRef type = arg["type"];

        LuaRef width = arg["width"];
        LuaRef height = arg["height"];

        requireNonNil(name, L, "createObject: name required");
        requireNonNil(type, L, "createObject: type required");
        requireNonNil(pos,L, "createObject: pos required");

        Vec2 posV = getVec2FromTable(pos);

        map<string,string> m;
        
        if(!props.isNil() && props.isTable()){
            m = getStringMapFromTable(props, L);
        }
        m["name"] = name.tostring();
        m["type"] = type.tostring();
        
        m[lauArgTag] = "true";
        
        ValueMap objArg = GObject::makeValueMapArg(posV,m);
        
        //Dimensions, if included for dynamically sized objects, must be converted to pixel space to match existing interface. Otherwise provide 0 for dimension for objects that expect it to be there.
        
        objArg["width"] = width.isNumber() ? Value(getFloat(width)*App::pixelsPerTile) : Value(0.0);
        objArg["height"] = height.isNumber() ? Value(getFloat(height)*App::pixelsPerTile) : Value(0.0);
        
        space->addObject(objArg);
        
        return 0;
    }
    
    int removeObject(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 1){
            error(L, "removeObject: single string required.");
            return 0;
        }

        LuaRef name(L);
        name.pop(L);

        GScene::getSpace()->removeObject(name.tostring());
        
        return 0;
    }
    
    int runScene(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 1){
            error(L, "runScene: single string required.");
            return 0;
        }
        
        LuaRef name(L);
        name.pop(L);
        
        try{
            GScene::runScene(name.tostring());
        }
        catch(exception){
            error(L, "runScene: " + name.tostring() + " not found");
        }
        
        return 0;
    }
    
    int castSpell(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 2){
            error(L, "castSpell: two strings required.");
            return 0;
        }

        LuaRef spell(L);
        spell.pop(L);

        LuaRef caster(L);
        caster.pop(L);

        GSpace* space = GScene::getSpace();
        if(!space){
            log("castSpell: cannot cast spell in this scene.");
            return 0;
        }
        
        GObject* obj = space->getObject(caster.tostring());
        Spellcaster* sc = dynamic_cast<Spellcaster*>(obj);
        
        if(!sc){
            log("castSpell: %s is not a Spellcaster.", caster.tostring().c_str());
            return 0;
        }
        
        sc->cast(spell.tostring());
        return 0;
    }
    
    int stopSpell(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 1){
            error(L, "stopSpell: one string required.");
            return 0;
        }

        LuaRef caster(L);
        caster.pop(L);

        GSpace* space = GScene::getSpace();
        if(!space){
            log("stopSpell: cannot use in this scene.");
            return 0;
        }
        
        GObject* obj = space->getObject(caster.tostring());
        Spellcaster* sc = dynamic_cast<Spellcaster*>(obj);
        
        if(!sc){
            log("stopSpell: %s is not a Spellcaster.", caster.tostring().c_str());
            return 0;
        }
        
        sc->stop();
        return 0;
    }
    
    int getObjectCount(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 0){
            error(L, "getObjectCount: no argument expected.");
            return 0;
        }

        GSpace* space = GScene::getSpace();
        if(!space){
            log("getObjectCount: cannot use in this scene.");
            return 0;
        }

        log("%d objects.", space->getObjectCount());
        
        return 0;
    }
    
    #define install(name) installFunction(name, #name)
    
    void Inst::installApi()
    {
        installFunction(luaLog,"log");
        install(createObject);
        install(runScene);
        install(removeObject);
        install(castSpell);
        install(stopSpell);
        install(getObjectCount);
    }
}