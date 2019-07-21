//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "Agent.hpp"
#include "AI.hpp"
#include "AIMixins.hpp"
#include "AIUtil.hpp"
#include "App.h"
#include "FileIO.hpp"
#include "GObject.hpp"
#include "GScene.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "PlayScene.hpp"

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

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }
    
    void runscript(string name){
        App::lua->runFile("scripts/"+name+".lua");
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
            log("Lua Inst created: %s.", name.c_str());        
    }
    
    Inst::~Inst()
    {
        lua_close(state);
        
        if(logInst)
            log("Lua Inst closed: %s.", name.c_str());
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
        
    void Inst::runString(const string& str)
    {        
        int error = luaL_dostring(state, str.c_str());
        
        if(error)
            log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
    {
        luaL_loadfile(state, io::getRealPath(path).c_str());
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
    
#define addFuncSame(x) addFunction(#x, &__cls::x)

    void Inst::installApi()
    {
		getGlobalNamespace(state)

		.beginClass<App>("App")
			.addStaticFunction("runOverworldScene", static_cast<GScene*(*)(string,string)>(&App::runOverworldScene))
			.addStaticFunction("getCrntScene", &App::getCrntScene)
			.addStaticFunction("getCrntState", &App::getCrntState)
			.addStaticFunction("printGlDebug", &App::printGlDebug)
#if USE_TIMERS
			.addStaticFunction("printTimerInfo", &App::printTimerInfo)
			.addStaticFunction("setLogTimers", &App::setLogTimers)
#endif
			.addStaticFunction("setFullscreen", &App::setFullscreen)
			.addStaticFunction("setVsync", &App::setVsync)
			.addStaticFunction("setMultithread", &App::setMultithread)
			.addStaticFunction("setResolution", &App::setResolution)
			.addStaticFunction("setFramerate", &App::setFramerate)
			.addStaticFunction("setPlayer", &App::setPlayer)
			.addStaticFunction("setUnlockAllEquips", &App::setUnlockAllEquips)
			.addStaticFunction("loadProfile", &App::loadProfile)
			.addStaticFunction("saveProfile", &App::saveProfile)

			.addStaticFunction("clearAllKeys", &App::clearAllKeys)
			.addStaticFunction("clearKeyAction", &App::clearKeyAction)
			.addStaticFunction("addKeyAction", &App::addKeyAction)
#if use_gamepad
			.addStaticFunction("clearAllButtons", &App::clearAllButtons)
			.addStaticFunction("clearButtonAction", &App::clearButtonAction)
			.addStaticFunction("addButtonAction", &App::addButtonAction)
#endif
		.endClass()

#define __cls AttributeSystem
		.beginClass<AttributeSystem>("AttributeSystem")
			.addFunction("getByName", &AttributeSystem::get)
			.addFunction("setByName", static_cast<void(AttributeSystem::*)(string, float)>(&AttributeSystem::set))
			.addFunction("getByID", &AttributeSystem::operator[])
			.addFunction("setByID", &AttributeSystem::_set)
			.addFunction("modifyAttribute", static_cast<void(AttributeSystem::*)(Attribute, float)>(&AttributeSystem::modifyAttribute))
			.addFuncSame(setFullHP)
			.addFuncSame(setFullMP)
			.addFuncSame(setFullStamina)
		.endClass()

#undef __cls

		.beginClass<GObject>("GObject")
			.addFunction("cast", &GObject::cast)
			.addFunction("getAngle", &GObject::getAngle)
			.addFunction("getAngularVel", &GObject::getAngularVel)
			.addFunction("getPos", &GObject::getPos)
			.addFunction("getVel", &GObject::getVel)
			.addFunction("setAngle", &GObject::setAngle)
			.addFunction("setAngularVel", &GObject::setAngularVel)
			.addFunction("setPos", &GObject::setPos)
			.addFunction("setVel", &GObject::setVel)
			.addFunction("setSpriteShader", &GObject::setSpriteShader)
			.addFunction("setVel", &GObject::setPos)
			.addFunction("stopSpell", &GObject::stopSpell)

			.addFunction("addThread", &GObject::addThread)
			.addFunction("printFSM", &GObject::printFSM)
			.addFunction("removeThreadByName", static_cast<void(GObject::*)(const string&)>(&GObject::removeThread))

		.endClass()
#define __cls Agent
		.deriveClass<Agent,GObject>("Agent")
			.addFuncSame(getAttributeSystem)
		.endClass()
#undef __cls

		.beginClass<GScene>("GScene")
			.addStaticFunction("runScene", &GScene::runScene)
			.addStaticFunction("runSceneWithReplay", &GScene::runSceneWithReplay)
			.addStaticData("suppressGameOver", &GScene::suppressGameOver)
			.addFunction("createDialog", static_cast<void(GScene::*)(const string&,bool)>(&GScene::createDialog))
			.addFunction("getSpace", &GScene::getSpace)
			.addFunction("setPaused", &GScene::setPaused)
			.addFunction("stopDialog", &GScene::stopDialog)
			.addFunction("teleportToDoor", &GScene::teleportToDoor)
			.addFunction("setRoomVisible", &GScene::setRoomVisible)
			.addFunction("unlockAllRooms", &GScene::unlockAllRooms)
		.endClass()

		.deriveClass<PlayScene,GScene>("PlayScene")
			.addFunction("saveReplayData", &PlayScene::saveReplayData)
		.endClass()

		.beginClass<GSpace>("GSpace")
			.addFunction("createObject", static_cast<gobject_ref (GSpace::*)(const ValueMap&)>(&GSpace::createObject))
			.addFunction("getFrame", &GSpace::getFrame)
			.addFunction("getObjectByName", static_cast<GObject*(GSpace::*)(const string&) const>(&GSpace::getObject))
			.addFunction("getObjectAsAgent", &GSpace::getObjectAs<Agent>)
			.addFunction("getObjectCount", &GSpace::getObjectCount)
			.addFunction("getObjectNames", &GSpace::getObjectNames)
			.addFunction("getUUIDNameMap", &GSpace::getUUIDNameMap)
			.addFunction("isObstacle", &GSpace::isObstacle)
			.addFunction("removeObject", static_cast<void(GSpace::*)(const string&)>(&GSpace::removeObject))
		.endClass()
			
		.beginClass<GState>("GState")
			.addFunction("addItem", &GState::addItem)
			.addFunction("hasItem", &GState::hasItem)
			.addFunction("registerChamberCompleted", &GState::_registerChamberCompleted)
			.addFunction("registerUpgrade", &GState::_registerUpgrade)
			.addFunction("setUpgradeLevels", &GState::setUpgradeLevels)
		.endClass()

		.beginClass<HUD>("HUD")
			.addFunction("setMansionMode", &HUD::setMansionMode)
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
			.addFunction("distanceToTarget", static_cast<SpaceFloat(*)(const GObject*, const GObject*)>(&ai::distanceToTarget))
			.addFunction("viewAngleToTarget", &ai::viewAngleToTarget)
		.endNamespace()
		;
    }

	string Inst::getSerialized(const string& name) {
		return callOneReturn("get_serialized", makeArgs(name), "");
	}

	void Inst::setSerialized(const string& name, const string& val) {
		call("set_serialized", makeArgs(name, val));
	}

	string Inst::callSerialized(const string& name, const string& args) {
		return callOneReturn("call_serialized", makeArgs(name, args), "");
	}

}
