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

	#define get_prop(cls,func) #func, sol::property(&cls::get_ ## func)
	#define rw_prop(cls,func) #func, sol::property(&cls::get_ ## func, &cls::set_ ## func)

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
		void addApp();
		void addGObject();
		void addGSpace();
		void addMagic();
		void addScene();
		void addStructures();
		void addTypes();
		void addUtil();

        void installApi();
        void loadLibraries();
        
        void runString(const string& str);
        void runFile(const string& path);
                
        void callNoReturn(const string& name);
        void callIfExistsNoReturn(const string& name);

		template<typename T>
		inline T getEnum(const string& clsName, const string& s, T _default)
		{
			sol::object obj = _state[clsName][s];

			if (obj) {
				return obj.as<T>();
			}
			else {
				return _default;
			}
		}

		inline sol::function getFunction(const string& name) { return _state[name]; }

		sol::state _state;
    };    
}

#endif /* Lua_hpp */
