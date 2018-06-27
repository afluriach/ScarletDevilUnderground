//
//  Lua.hpp
//  Koumachika
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

using namespace luabridge;

#include "LuaConvert.h"

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
        static const bool logInst = false;
        
        static unordered_map<string, Inst*> instances;
        
        static vector<pair<string,string>> commandQueue;
        static mutex queueLock;
        static void runCommands();
        static void addCommand(const string& target, const string& script);
    
        void installApi();
        void installWrappers();
        void loadLibraries();
        void installFunction(lua_CFunction func, const string& name);
        void setGlobal(LuaRef ref, const string& name);
        
        template<typename T>
        void setGlobal(T t, const string& name){
            setGlobal(Lua::convert<T>::convertToLua(t, state), name);
        }
        
        bool globalExists(const string& name);
        
        void runString(const string& str);
        //This supports Lua text as well as object files. It uses loadfile directly
        //rather than passing the file's contents as a string.
        void runFile(const string& path);
        
        template<typename T>
        vector<LuaRef> makeArgs(T t)
        {
            return vector<LuaRef>{convert<T>::convertToLua(t,state)};
        }
        
        template<typename T, typename U>
        vector<LuaRef> makeArgs(T t, U u)
        {
            return vector<LuaRef>{
                convert<T>::convertToLua(t,state),
                convert<U>::convertToLua(u,state)
            };
        }
        
        vector<LuaRef> call(const string& name, const vector<LuaRef>& params);
        void callNoReturn(const string& name);
        vector<LuaRef> callIfExists(const string& name, const vector<LuaRef>& params);
        void callIfExistsNoReturn(const string& name);
        void callIfExistsNoReturn(const string& name, const vector<LuaRef>& params);
        
        template<typename T>
        inline T callOneReturn(const string& name, vector<LuaRef> args, T defaultReturn){
            vector<LuaRef> result = call(name, args);
            if(!result.empty())
                return result[0];
            else
                return defaultReturn;
        }

        inline string getSerialized(const string& name){
            return callOneReturn("get_serialized", makeArgs(name), "");
        }
        
        inline void setSerialized(const string& name, const string& val){
            call("set_serialized", makeArgs(name, val));
        }
        
        inline string callSerialized(const string& name, const string& args){
            return callOneReturn("call_serialized", makeArgs(name, args), "");
        }
        
        //Helper for making Lua data, since LuaRef requires the lua state.
        template<typename T>
        LuaRef makeRef(T data)
        {
            return LuaRef(state,data);
        }
        lua_State *state;
    };

    void replThreadMain(Inst * inst);
    
}

#endif /* Lua_hpp */