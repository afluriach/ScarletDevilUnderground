//
//  Lua.hpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

using namespace luabridge;

namespace Lua
{
    //Raise Lua exception
    void error(lua_State* L, const string& msg);
    
    int luaContextPanic(lua_State* L);
        
    //C API function, runs a script using its simplied name in the app's Lua context.
    void runscript(string name);

    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        Inst(const string& name);
        ~Inst();
        
        const string name;
        
        static const vector<string> luaIncludes;
        static const bool catchLuaPanic = false;
        
        static unordered_map<string, Inst*> instances;
    
        void installApi();
        void installWrappers();
        void loadLibraries();
        void installFunction(lua_CFunction func, const string& name);
        void setGlobal(LuaRef ref, const string& name);
        bool globalExists(const string& name);
        
        void runString(const string& str);
        //This supports Lua text as well as object files. It uses loadfile directly
        //rather than passing the file's contents as a string.
        void runFile(const string& path);
        
        vector<LuaRef> call(const string& name, const vector<LuaRef>& params);
        vector<LuaRef> callIfExists(const string& name, const vector<LuaRef>& params);
        
        //Helper for making Lua data, since LuaRef requires the lua state.
        template<typename T>
        LuaRef makeRef(T data)
        {
            return LuaRef(state,data);
        }
        lua_State *state;
    };
    
}

#endif /* Lua_hpp */
