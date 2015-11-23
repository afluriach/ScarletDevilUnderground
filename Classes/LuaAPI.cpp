//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "GObject.hpp"

#include "LuaAPI.hpp"

using namespace std;

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
            
            cocos2d::log("%s,%s", key.c_str(), value.c_str());
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
            cocos2d::log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
    {
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
    
    int createObject(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        GSpaceScene* scene = dynamic_cast<GSpaceScene*>(GScene::crntScene);
        if(!scene){
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
        LuaRef dim = arg["dim"];
        LuaRef props = arg["props"];
        
        LuaRef name = arg["name"];
        LuaRef type = arg["type"];

        if(name.isNil()){
            error(L, "createObject: name required");
            return 0;
        }
        
        if(type.isNil()){
            error(L, "createObject: type required");
            return 0;
        }

        
        if(pos.isNil()){
            error(L, "createObject: pos required");
            return 0;
        }

        if(dim.isNil()){
            error(L, "createObject: dim required");
            return 0;
        }

        if(pos["x"].isNil() || !pos["x"].isNumber()){
            error(L, "createObject: pos.x invalid");
            return 0;
        }

        if(pos["y"].isNil() || !pos["y"].isNumber()){
            error(L, "createObject: pos.y invalid");
            return 0;
        }
        if(dim["width"].isNil() || !dim["width"].isNumber()){
            error(L, "createObject: dim.width invalid");
            return 0;
        }
        if(dim["height"].isNil() || !dim["height"].isNumber()){
            error(L, "createObject: dim.height invalid");
            return 0;
        }

        map<string,string> m;
        
        Vec2 posV(pos["x"].cast<float>(), pos["y"].cast<float>());
        Vec2 dimV(dim["width"].cast<float>(), dim["height"].cast<float>());
        
        if(props.isTable()){
            m = getStringMapFromTable(props, L);
        }
        m["name"] = name.tostring();
        m["type"] = type.tostring();

        
        ValueMap objArg = GObject::makeValueMapArg(posV,dimV,m);
        
        scene->gspace.addObject(objArg);
        
        return 0;
    }
    
    int runScene(lua_State* L)
    {
        int nArgs = lua_gettop(L);
        
        if(nArgs != 1){
            cocos2d::log("runScene: single string required.");
            return 0;
        }
        
        LuaRef name(L);
        name.pop(L);
        
        GScene::runScene(name.tostring());

        return 0;
    }
    
    void Inst::installApi()
    {
        installFunction(log,"log");
        installFunction(createObject,"createObject");
        installFunction(runScene,"runScene");
    }
}