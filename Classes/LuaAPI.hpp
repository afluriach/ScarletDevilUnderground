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
    static const string lauArgTag = "luaScript";

    class lua_runtime_error : public runtime_error
    {
    public:
        inline lua_runtime_error(string what) : runtime_error(what) {}
        inline lua_runtime_error(const string& name, int arg, const string& msg)
        : lua_runtime_error(StringUtils::format("%s, argument %d: %s", name.c_str(), arg, msg.c_str())){}

    };
    
    class lua_type_error : public lua_runtime_error
    {
    public:
        inline lua_type_error(string what) : lua_runtime_error(what) {}
        inline lua_type_error(const string& name, int arg, const string& msg)
        : lua_runtime_error(name, arg, msg){}

    };

    //Raise Lua exception
    void error(lua_State* L, const string& msg);
    
    int luaContextPanic(lua_State* L);
    
    map<string,string> getStringMapFromTable(LuaRef table);
    
    list<LuaRef> getArgs(lua_State* L);
    
    //C API function, runs a script using its simplied name in the app's Lua context.
    void runscript(string name);

    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        Inst();
        ~Inst();
        
        static const vector<string> luaIncludes;
    
        void installApi();
        void installWrappers();
        void loadLibraries();
        void installFunction(lua_CFunction func, const string& name);
        void setGlobal(LuaRef ref, const string& name);
        bool globalExists(const string& name);
        
        void runString(const string& str);
        void runFile(const string& path);
        
        //Strictly speaking, this supports Lua text as well as object files. It uses loadfile directly
        //rather than passing the file's contents as a string.
        void runObjectFile(const string& path);
        vector<LuaRef> call(const string& name, const vector<LuaRef>& params);
        vector<LuaRef> callIfExists(const string& name, const vector<LuaRef>& params);
        
        //Helper for making Lua data, since LuaRef requires the lua state.
        template<typename T>
        LuaRef makeRef(T data)
        {
            return LuaRef(state,data);
        }
    private:
        lua_State *state;
    };
    
    template<typename K, typename V>
    map<K,V> getMapFromTable(LuaRef table)
    {
        map<K,V> result;
        
        for(auto it = Iterator(table); !it.isNil(); ++it)
        {
            K key;
            V value;
            try{
                LuaRef luakey = it.key();
                key = luakey.cast<K>();
            }catch(exception ex){
                throw lua_type_error("Key of wrong type");
            }
            try{
                LuaRef luavalue = it.value();
                value = luavalue.cast<V>();
            } catch(exception ex){
                throw lua_type_error("Value of wrong type.");
            }
            result[key] = value;
        }
        return result;
    }
    
    function<void()> makeFunctorFromLuaFunction(LuaRef ref);
}

#endif /* Lua_hpp */
