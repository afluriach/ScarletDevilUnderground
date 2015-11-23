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

//Lua API methods
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
    
    void Inst::installApi()
    {
        installFunction(log,"log");
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
            cocos2d::log("runString Lua error.");
    }
    
    void Inst::runFile(const string& path)
    {
        runString(loadTextFile(path));
    }

     vector<shared_ptr<Data>> Inst::call(const string& name, const vector<shared_ptr<Lua::Data>>& params)
    {
        int top = lua_gettop(state);
        
        lua_pushglobaltable(state);
        lua_getfield(state, -1, name.c_str());
        //Remove global table after pushing function to call
        lua_remove(state, -2);
        
        foreach(shared_ptr<Lua::Data> d, params)
        {
            d->push(state);
        }
        
        lua_call(state, params.size(), LUA_MULTRET);
        
        int nResults = lua_gettop(state) - top;
        vector<shared_ptr<Data>> results;
        
        for(int i=0;i<nResults; ++i){
            results.push_back(Data::pop(state));
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

}