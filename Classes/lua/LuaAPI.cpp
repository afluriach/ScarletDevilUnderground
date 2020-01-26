//
//  Lua.cpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "FileIO.hpp"
#include "LuaAPI.hpp"

namespace Lua{

const vector<string> Inst::luaIncludes = {
	"util",
	"30log-global",
	"serpent",
	"ai"
};

    Inst::Inst(const string& name) : name(name)
    {
		_state.open_libraries(
			sol::lib::base,
			sol::lib::math,
			sol::lib::string,
			sol::lib::table,
			sol::lib::utf8
		);

        installApi();
        loadLibraries();
        
        if(logInst)
            log("Lua Inst created: %s.", name.c_str());        
    }
    
    Inst::~Inst()
    {        
        if(logInst)
            log("Lua Inst closed: %s.", name.c_str());
    }
    
    void Inst::loadLibraries()
    {
		for(auto const& s : luaIncludes){
            runFile("scripts/"+s+".lua");
        }
    }
        
    void Inst::runString(const string& str)
    {
		try {
			_state.script(str);
		}
		catch (sol::error e){
			log("script %s error: %s", name, e.what());
		}
    }
    
    void Inst::runFile(const string& path)
    {
		runString(io::loadTextFile(path));
    }
    
    void Inst::callIfExistsNoReturn(const string& name)
    {
		sol::function f = _state[name];

		if (f) f();
    }
        
    void Inst::callNoReturn(const string& name)
    {
		_state[name]();
    };
    
    void Inst::installApi()
    {
		auto util = _state.create_table();
		_state["util"] = util;

		//Lua does not support passing primitves by reference, thus sol does not
		//support wrapping the C++ version of these functions.
		util["timerDecrement"] = [](SpaceFloat input) -> SpaceFloat {
			SpaceFloat result = input;
			timerDecrement(result);
			return result;
		};
		util["timerIncrement"] = [](SpaceFloat input) -> SpaceFloat {
			SpaceFloat result = input;
			timerIncrement(result);
			return result;
		};

		addAI();
		addApp();
		addGObject();
		addGSpace();
		addMagic();
		addScene();
		addTypes();
	}
}
