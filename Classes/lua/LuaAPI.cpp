//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "scenes.h"
#include "util.h"

//Copied from ltm.h.
//#define ttypename(x)	luaT_typenames_[(x) + 1]
//extern const char *const luaT_typenames_[LUA_NUMTAGS+2];

namespace Lua{

const vector<string> Inst::luaIncludes = boost::assign::list_of
    ("util")
    ("class")
    ("math")
    ("serpent")
    ("repl")
    ("gobject")
    ("ai")
;

unordered_map<string, Inst*> Inst::instances;

vector<pair<string,string>> Inst::commandQueue;
mutex Inst::queueLock;

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }
    
    void runscript(string name){
        app->lua.runFile("scripts/"+name+".lua");
    }

    void requireType(lua_State* L, const string& name, LuaRef ref, int typeID)
    {
        if(!ref.isType(typeID)){
            error(
                L,
                StringUtils::format(
                    "%s: expected %s, got %s.",
                    name.c_str(),
                    ttypename(typeID),
                    ref.getTypeName()
                )
            );
        }
    }
    
    void replThreadMain(Inst * inst)
    {
        inst->call("open_repl", vector<LuaRef>());
    }

//Lua API methods

    Inst::Inst(const string& name) : name(name)
    {
        state = luaL_newstate();
        //Load standard libraries
        luaL_openlibs(state);
        installApi();
        installWrappers();
        loadLibraries();
        
        if(catchLuaPanic)
            lua_atpanic(state, luaContextPanic);

        if(logInst)
            cocos2d::log("Lua Inst created: %s.", name.c_str());
        
        if(instances.find(name) != instances.end())
            cocos2d::log("Lua Inst with duplicate name: %s.", name.c_str());
        instances[name] = this;
    }
    
    Inst::~Inst()
    {
        lua_close(state);
        instances.erase(name);
        
        if(logInst)
            cocos2d::log("Lua Inst closed: %s.", name.c_str());
    }
    
    void Inst::loadLibraries()
    {
        BOOST_FOREACH(auto s, luaIncludes){
            runFile("scripts/"+s+".lua");
        }
    }
    
    int Inst::dispatch(lua_State* L)
    {
        string fname = lua_tostring(L,lua_upvalueindex(1));
        auto it = cfunctions.find(fname);
        
        if(it != cfunctions.end()){
            return it->second(L);
        }
        else{
            log("Inst::dispatch: %s not found!", fname.c_str());
            return 0;
        }
    }
    
    void Inst::installNameFunction(const string& name)
    {
        lua_pushstring(state, name.c_str());
        lua_pushcclosure(state, &Inst::dispatch, 1);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::installFunction(lua_CFunction func, const string& name)
    {
        lua_pushcfunction(state, func);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::setGlobal(LuaRef ref, const string& name)
    {
        ref.push(state);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::addCommand(const string& target, const string& script)
    {
        queueLock.lock();
        commandQueue.push_back(pair<string,string>(target,script));
        queueLock.unlock();
    }
    
    void Inst::runCommands()
    {
        queueLock.lock();
        
        BOOST_FOREACH(auto command, commandQueue)
        {
            auto it = Inst::instances.find(command.first);
        
            if(it != Inst::instances.end()){
                it->second->runString(command.second);
            }
            else{
                cocos2d::log("runCommands: instance %s not found.", command.first.c_str());
            }
        }
        
        commandQueue.clear();
        
        queueLock.unlock();
    }
    
    void Inst::runString(const string& str)
    {        
        int error = luaL_dostring(state, str.c_str());
        
        if(error)
            log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
    {
        luaL_loadfile(state, getRealPath(path).c_str());
        docall(state, 0, LUA_MULTRET);
    }

    bool Inst::globalExists(const string& name)
    {
        lua_getglobal(state, name.c_str());
        bool result = !lua_isnil(state, -1);
        lua_pop(state, 1);
        
        return result;
    }

    vector<LuaRef> Inst::callIfExists(const string& name, const vector<LuaRef>& params)
    {
        if(!globalExists(name))
            return vector<LuaRef>();
        
        return call(name, params);
    }
    
    void Inst::callIfExistsNoReturn(const string& name)
    {
        if(!globalExists(name))
            return;
        
        return callNoReturn(name);
    }
    
    void Inst::callIfExistsNoReturn(const string& name, const vector<LuaRef>& params)
    {
        if(!globalExists(name))
            return;
        
        call(name, params);
    }
    
    void Inst::callNoReturn(const string& name)
    {
        lua_pushglobaltable(state);
        lua_getfield(state, -1, name.c_str());
        //Remove global table after pushing function to call
        lua_remove(state, -2);
        
        try{
        
            int error = docall(state, 0, 0);

            if(error)
            {
                log("Lua::Inst::call: %s, error: %s", name.c_str(), lua_tostring(state,-1));
            }
        }
        catch(lua_runtime_error ex){
            log("Lua::Inst, %s: runtime exception %s.", name.c_str(), ex.what());
        }
    };

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
        
        try{
        
            int error = docall(state, params.size(), LUA_MULTRET);

            if(error)
            {
                log("Lua::Inst::call: %s, error: %s", name.c_str(), lua_tostring(state,-1));
                return vector<LuaRef>();
            }
            else
            {
                int nResults = lua_gettop(state) - top;
                vector<LuaRef> results;
                
                for_irange(i,0,nResults){
                    LuaRef ref(state);
                    ref.pop(state);
                    results.push_back(ref);
                }
                reverse(results.begin(), results.end());
                return results;
            }
        }
        catch(lua_runtime_error ex){
            log("Lua::Inst, %s: runtime exception %s.", name.c_str(), ex.what());
            return vector<LuaRef>();
        }
    };
    
    #define NARGS int nArgs = lua_gettop(L);

    char errorbuf[128];
    #define check_args(target)\
    if(nArgs != target){ \
        snprintf(errorbuf, 128, "%s: %d parameters required, %d found", crntFuncName, target, nArgs); \
        error(L, errorbuf); \
        return 1; \
    }
    
    #define get_gspace \
    GSpace* gspace = app->space; \
    if(!gspace){ \
        snprintf(errorbuf, 128, "%s: cannot use in current scene.", crntFuncName); \
        error(L, errorbuf); \
        return 1; \
    } \

    #define arg(name) \
    LuaRef name(L); \
    name.pop(L);
    
    //Create LuaRef from value stored in table under the same name.
    #define ref_from_table(table, name) LuaRef name = table[#name];
    
    #define func(name) \
    int name(lua_State* L) \
    { \
    const char* crntFuncName = #name;
    
    #define push_error(errorMsg) {\
    snprintf(errorbuf, 128, "%s: %s", crntFuncName, errorMsg); \
    error(L, errorbuf); \
    return 1; }

    #define c_string(string_expr) string(string_expr).c_str()
        
    //Lua API functions:
    
    func(luaContextPanic)

        throw lua_runtime_error("Lua Panic.");
        
        return 0;
    }

    #define install(name) installFunction(name, #name)
    
    void Inst::installApi()
    {
		getGlobalNamespace(state)

		.beginClass<App>("App")
			.addStaticData("width", &App::width)
			.addStaticData("height", &App::height)
			.addStaticData("app", &app)
			.addStaticFunction("setFullscreen", &App::setFullscreen)
			.addStaticFunction("setResolution", &App::setResolution)
			.addStaticFunction("setFramerate", &App::setFramerate)
			.addFunction("setPlayer", &App::setPlayer)
			.addData("space", &App::space)
		.endClass()

		.beginClass<GScene>("GScene")
			.addStaticFunction("runScene", &GScene::runScene)
			.addStaticFunction("runSceneWithReplay", &GScene::runSceneWithReplay)
			.addStaticData("crntScene", &GScene::crntScene)
			.addStaticData("suppressGameOver", &GScene::suppressGameOver)
			.addFunction("setPaused", &GScene::setPaused)
		.endClass()

		.beginClass<GSpace>("GSpace")
			.addFunction("getFrame", &GSpace::getFrame)
			.addFunction("getObjectCount", &GSpace::getObjectCount)
			.addFunction("getUUIDNameMap", &GSpace::getUUIDNameMap)
		.endClass()	
			
		.beginClass<HUD>("HUD")
			.addFunction("showHealth", &HUD::showHealth)
		.endClass();
    }
}
