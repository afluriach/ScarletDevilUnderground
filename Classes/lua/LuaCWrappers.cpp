//
//  LuaCWrappers.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#include "AI.hpp"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "GState.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "LuaWrap.h"
#include "Player.hpp"
#include "PlayScene.hpp"

namespace Lua{

void printMap(unordered_map<string,string> m)
{
    for(auto it = m.begin(); it != m.end(); ++it){
        log("%s : %s", it->first.c_str(), it->second.c_str());
    }
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

void Inst::installWrappers()
{
    for(auto it = cfunctions.begin(); it != cfunctions.end(); ++it)
    {
        installNameFunction(it->first);
    }    
}

const unordered_map<string, function<int(lua_State*)>> Inst::cfunctions = {
	make_wrapper_same(printMap),
	make_wrapper_same(runscript),

	make_wrapper_same(toDirection),
	make_wrapper_same(stringToDirection)
};

}
