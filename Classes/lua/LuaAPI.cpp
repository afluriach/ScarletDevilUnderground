//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "AIMixins.hpp"
#include "App.h"
#include "GObject.hpp"
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

const vector<string> Inst::luaIncludes = {
	"util",
	"class",
	"math",
	"serpent",
	"repl",
	"gobject",
	"ai"
};

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
		for(auto const& s : luaIncludes){
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
        
        for(auto const &command: commandQueue)
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
        
        for(auto const& r: params)
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
    
    void Inst::installApi()
    {
		getGlobalNamespace(state)

		.beginClass<App>("App")
			.addStaticData("width", &App::width)
			.addStaticData("height", &App::height)
			.addStaticData("app", &app)
			.addStaticFunction("printGlDebug", &App::printGlDebug)
			.addStaticFunction("setFullscreen", &App::setFullscreen)
			.addStaticFunction("setResolution", &App::setResolution)
			.addStaticFunction("setFramerate", &App::setFramerate)
			.addFunction("setPlayer", &App::setPlayer)
		.endClass()

		.beginClass<GObject>("GObject")
			.addFunction("cast", static_cast<void(GObject::*)(const string&, const ValueMap&)>(&GObject::cast))
			.addFunction("setPos", &GObject::setPos)
			.addFunction("setSpriteShader", &GObject::setSpriteShader)
			.addFunction("setVel", &GObject::setPos)
			.addFunction("stopSpell", &GObject::stopSpell)

		.endClass()

		.deriveClass<StateMachineObject, GObject>("StateMachineObject")
			.addFunction("addThread", &StateMachineObject::addThread)
			.addFunction("printFSM", &StateMachineObject::printFSM)
			.addFunction("removeThreadByID", static_cast<void(StateMachineObject::*)(unsigned int)>(&StateMachineObject::removeThread))
			.addFunction("removeThreadByName", static_cast<void(StateMachineObject::*)(const string&)>(&StateMachineObject::removeThread))
		.endClass()

		.beginClass<GScene>("GScene")
			.addStaticFunction("runScene", &GScene::runScene)
			.addStaticFunction("runSceneWithReplay", &GScene::runSceneWithReplay)
			.addStaticData("crntScene", &GScene::crntScene)
			.addStaticData("suppressGameOver", &GScene::suppressGameOver)
			.addFunction("createDialog", &GScene::createDialog)
			.addFunction("getSpace", &GScene::getSpace)
			.addFunction("setPaused", &GScene::setPaused)
			.addFunction("stopDialog", &GScene::stopDialog)
		.endClass()

		.beginClass<GSpace>("GSpace")
			.addFunction("createObject", static_cast<gobject_ref (GSpace::*)(const ValueMap&)>(&GSpace::createObject))
			.addFunction("getFrame", &GSpace::getFrame)
			.addFunction("getObjectByName", static_cast<GObject*(GSpace::*)(const string&) const>(&GSpace::getObject))
			.addFunction("getObjectAsSMO", &GSpace::getObjectAs<StateMachineObject>)
			.addFunction("getObjectCount", &GSpace::getObjectCount)
			.addFunction("getObjectNames", &GSpace::getObjectNames)
			.addFunction("getUUIDNameMap", &GSpace::getUUIDNameMap)
			.addFunction("isObstacle", &GSpace::isObstacle)
			.addFunction("removeObject", static_cast<void(GSpace::*)(const string&)>(&GSpace::removeObject))
		.endClass()
			
		.beginClass<HUD>("HUD")
			.addFunction("showHealth", &HUD::showHealth)
			.addFunction("setObjectiveCounter", &HUD::setObjectiveCounter)
			.addFunction("setObjectiveCounterVisible", &HUD::setObjectiveCounterVisible)
		.endClass()

		.beginClass<SpaceVect>("SpaceVect")
			.addConstructor<void(*)(double,double)>()
			.addData("x", &SpaceVect::x)
			.addData("y", &SpaceVect::y)
		.endClass()
			
		.beginNamespace("ai")
			.addFunction("applyDesiredVelocity",&ai::applyDesiredVelocity)
			.addFunction("seek", &ai::seek)
			.addFunction("flee", &ai::flee)
			.addFunction("isFacingTarget", &ai::isFacingTarget)
			.addFunction("isFacingTargetsBack", &ai::isFacingTargetsBack)
			.addFunction("isLineOfSight", &ai::isLineOfSight)
			.addFunction("directionToTarget", &ai::directionToTarget)
			.addFunction("displacementToTarget", &ai::displacementToTarget)
			.addFunction("distanceToTarget", &ai::distanceToTarget)
			.addFunction("viewAngleToTarget", &ai::viewAngleToTarget)
		.endNamespace()
		;
    }
}
