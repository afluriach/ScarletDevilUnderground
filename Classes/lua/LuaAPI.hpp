//
//  Lua.hpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

namespace Lua
{
    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        Inst(const string& name);
        ~Inst();
        
        const string name;
        
        static const vector<string> luaIncludes;
        static const bool logInst = false;
        
        void installApi();
        void loadLibraries();
        
        void runString(const string& str);
        void runFile(const string& path);
                
        void callNoReturn(const string& name);
        void callIfExistsNoReturn(const string& name);        
	protected:
		sol::state _state;
    };    
}

#endif /* Lua_hpp */
