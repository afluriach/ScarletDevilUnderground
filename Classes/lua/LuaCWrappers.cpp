//
//  LuaCWrappers.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "App.h"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaWrap.h"
#include "Player.hpp"
#include "PlayScene.hpp"

namespace Lua{

unordered_map<string, Class> Class::classes;
bool Class::init = false;

void printMap(unordered_map<string,string> m)
{
    for(auto it = m.begin(); it != m.end(); ++it){
        log("%s : %s", it->first.c_str(), it->second.c_str());
    }
}

void addUpdate(function<void()> f, int order)
{
    GScene::crntScene->multiUpdate.insertWithOrder(
        [=]() -> void{
            f();
        },
        order
    );
}

void dostring_in_inst(string script, string inst_name)
{
    Inst::addCommand(inst_name, script);
}

void printGlDebug()
{
    //print the program, vertex, and fragment log for each custom shader.
    
    log("printGlDebug:");
    
    for(const string& name: App::shaderFiles){
        log("%s", name.c_str());
        GLProgram* p = GLProgramCache::getInstance()->getGLProgram(name);
        
        string logtext;
        
        logtext = p->getProgramLog();
        if(!logtext.empty())
            log("%s program log: \n%s", name.c_str(), logtext.c_str());

        logtext = p->getVertexShaderLog();
        if(!logtext.empty())
            log("%s vertex log: \n%s", name.c_str(), logtext.c_str());

        logtext = p->getFragmentShaderLog();
        if(!logtext.empty())
            log("%s fragment log: \n%s", name.c_str(), logtext.c_str());

    }
}

void save()
{
    GState::save();
}

bool saveCrntReplay(string filepath)
{
	return App::getCrntSceneAs<PlayScene>()->saveReplayData(filepath);
}

vector<string> getInventoryContents()
{
    auto registry = GState::crntState.itemRegistry;
    vector<string> items(registry.begin(), registry.end());
    
    return items;
}

shared_ptr<ai::Function> constructState(string funcName, string stateName, GSpace* space, ValueMap args)
{
	shared_ptr<ai::Function> state = ai::Function::constructState(stateName, space, args);
	if (!state) {
		log("%s: Unknown state class %s", funcName.c_str(), stateName.c_str());
		return nullptr;
	}

	return state;
}

///////////////////////////////////////////////////////////////////

#define make_wrapper(name,f) {name, wrap_cfunction(name,f)}
#define make_package_wrapper(ns,name) {#name, wrap_cfunction(#name, ns::name)}
#define make_wrapper_same(name) {#name, wrap_cfunction(#name,name)}

#define make_method_wrapper(cls, name) \
int name ## _wrapper(lua_State* L) \
{ \
    try{ \
        return methodWrapper(#cls "::" #name, &cls::name, L);\
    }catch(runtime_error err){ \
        Lua::error(L, err.what()); \
        return 1; \
    } \
}

#define install_wrapper(name) installFunction(name ## _wrapper, #name);
#define install_method_wrapper(cls,name) installFunction(name ## _wrapper, #cls "_" #name);
#define add_method(cls,name) Class::addMethod(#cls, #name, name ## _wrapper);

make_method_wrapper(GObject,getPos)
make_method_wrapper(GObject,setPos)
make_method_wrapper(GObject,getVel)
make_method_wrapper(GObject,setVel)
make_method_wrapper(GObject, setDirection)
make_method_wrapper(GObject,getUUID)
make_method_wrapper(GObject,getName)
make_method_wrapper(GObject,setScriptVal)
make_method_wrapper(GObject,getScriptVal)
make_method_wrapper(GObject,_callScriptVal)

void Class::makeClasses()
{
    Class::makeClass("GObject");
//    Class::makeClass("Spellcaster");
    
    add_method(GObject,getPos)
    add_method(GObject,setPos)
    add_method(GObject,getVel)
    add_method(GObject,setVel)
    add_method(GObject, setDirection)
    add_method(GObject,getUUID)
    add_method(GObject,getName)
    add_method(GObject,setScriptVal)
    add_method(GObject,getScriptVal)
    add_method(GObject,_callScriptVal)
}

void Inst::installWrappers()
{
    for(auto it = cfunctions.begin(); it != cfunctions.end(); ++it)
    {
        installNameFunction(it->first);
    }
    
    Class::installClasses(state);
}

const unordered_map<string, function<int(lua_State*)>> Inst::cfunctions = {
	make_wrapper_same(printMap),
	make_wrapper_same(addUpdate),
	make_wrapper_same(runscript),
	make_wrapper_same(dostring_in_inst),
	make_wrapper_same(printGlDebug),
	make_wrapper_same(save),
	make_wrapper_same(saveCrntReplay),
	make_wrapper_same(getInventoryContents),

	make_wrapper_same(toDirection),
	make_wrapper_same(stringToDirection),

	make_package_wrapper(ai,applyDesiredVelocity),
	make_package_wrapper(ai,seek),
	make_package_wrapper(ai,flee),

	make_package_wrapper(ai,isFacingTarget),
	make_package_wrapper(ai,isFacingTargetsBack),
	make_package_wrapper(ai,isLineOfSight),

	make_package_wrapper(ai,directionToTarget),
	make_package_wrapper(ai,displacementToTarget),
	make_package_wrapper(ai,distanceToTarget),
	make_package_wrapper(ai,viewAngleToTarget)

};

}
