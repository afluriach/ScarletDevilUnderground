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
#include "sol_util.hpp"

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
            log1("Lua Inst created: %s.", name.c_str());
    }
    
    Inst::~Inst()
    {        
        if(logInst)
            log1("Lua Inst closed: %s.", name.c_str());
    }
    
    void Inst::loadLibraries()
    {
		for(auto const& s : luaIncludes){
            runFile("scripts/"+s+".lua");
        }
    }
        
    void Inst::runString(const string& str)
    {
        auto error_handler = [this,str](lua_State*, sol::protected_function_result pfr) {
            sol::error lua_error = pfr;
            
            log1("runString: error running Lua string: \"%s\"", str);
            sol::printErrorMessage(_state);
            
            return pfr;
        };

        _state.safe_script(str, error_handler);
    }
    
    void Inst::runFile(const string& path)
    {
        auto error_handler = [this,path](lua_State*, sol::protected_function_result pfr) {
            sol::error lua_error = pfr;
            
            log1("runFile: error running Lua file: \"%s\"", path);
            sol::printErrorMessage(_state);
            
            return pfr;
        };

        string luaText = io::loadTextFile(path);

        if(luaText.empty()){
            log1("runFile: Warning, Lua script \"%s\" not found or is empty file.", path);
            return;
        }

        _state.safe_script(luaText, error_handler);
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

    void Inst::installSceneApi()
    {
        addScene();
    }
    
    void Inst::installApi()
    {
		addAI();
		addApp();
		addGObject();
		addGSpace();
		addMagic();
		addStructures();
		addTypes();
		addUtil();
	}
}
