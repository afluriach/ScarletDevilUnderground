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
	#define enum_entry(cls, x) { #x, cls::x }

	#define newType(x) _state.new_usertype<x>(#x);
	#define addFuncSame(v,x) v[#x] = &_cls::x;
	#define cFuncSame(v,x) v[#x] = &x;
	#define cFuncSameNS(v,ns,x) v[#x] = &ns::x;

    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        Inst(const string& name);
        ~Inst();
        
        const string name;
        
        static const vector<string> luaIncludes;
        static const bool logInst = false;
        
		void addAI();

        void installApi();
        void loadLibraries();
        
        void runString(const string& str);
        void runFile(const string& path);
                
        void callNoReturn(const string& name);
        void callIfExistsNoReturn(const string& name);

		inline sol::function getFunction(const string& name) { return _state[name]; }

		sol::state _state;
    };    
}

#endif /* Lua_hpp */
