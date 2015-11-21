//
//  Lua.hpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#ifndef Lua_hpp
#define Lua_hpp

#include <algorithm>
#include <memory>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "cocos2d.h"
#include "lua.hpp"
#include "util.h"

using namespace std;

namespace Lua
{

    //Abstract base representing a Lua object.
    class Data
    {
    public:
        virtual string toStr() const = 0;
    
        virtual void push(lua_State* L) = 0;
        static shared_ptr<Data> copy(lua_State* L);
        inline static shared_ptr<Data> pop(lua_State* L)
        {
            shared_ptr<Data> val = copy(L);
            lua_pop(L,1);
            return val;
        }
    };
    
    class Number : public Lua::Data
    {
    public:
        inline Number()
        {
            n = 0;
        }
        inline Number(double n) : n(n) {}
        
        double n;
        inline void push(lua_State* L)
        {
            lua_pushnumber(L, n);
        }
        inline virtual string toStr() const
        {
            return boost::lexical_cast<string>(n);
        }
    };
    
    class String : public Lua::Data
    {
    public:
        inline String()
        {
            s = "";
        }
        inline String(const string& s) : s(s) {}
        
        string s;
        inline void push(lua_State* L)
        {
            lua_pushstring(L, s.c_str());
        }
        inline virtual string toStr() const
        {
            return s;
        }
    };
    
    //Raise Lua exception
    void error(lua_State* L, const string& msg);
    
    //Lua API functions:
    int log(lua_State* L);


    //Wraps a VM instance and interfaces with it.
    class Inst
    {
    public:
        inline Inst()
        {
            state = luaL_newstate();
            //Load standard libraries
            luaL_openlibs(state);
            installApi();
        }
        inline ~Inst()
        {
            lua_close(state);
        }
        
        inline void installApi()
        {
            installFunction(log,"log");
        }
        
        inline void installFunction(lua_CFunction func, const string& name)
        {
            lua_pushcfunction(state, func);
            lua_setglobal(state, name.c_str());
        }
        
        inline void runString(const string& str)
        {        
            int error = luaL_dostring(state, str.c_str());
            
            if(error)
                cocos2d::log("runString Lua error.");
        }
        
        inline void runFile(const string& path)
        {
            runString(loadTextFile(path));
        }
    
        inline vector<shared_ptr<Data>> call(const string& name, const vector<shared_ptr<Lua::Data>>& params)
        {
            int top = lua_gettop(state);
            
            lua_pushglobaltable(state);
            lua_getfield(state, -1, name.c_str());
            //Remove global table after pushing function to call
            lua_remove(state, -2);
            
            foreach(shared_ptr<Lua::Data> d, params)
            {
                d->push(state);
            }
            
            lua_call(state, params.size(), LUA_MULTRET);
            
            int nResults = lua_gettop(state) - top;
            vector<shared_ptr<Data>> results;
            
            for(int i=0;i<nResults; ++i){
                results.push_back(Data::pop(state));
            }
            reverse(results.begin(), results.end());
            return results;
        };
    private:
        lua_State *state;
    };
}

#endif /* Lua_hpp */
