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
        Inst();
        ~Inst();
        
        void installApi();
        void installFunction(lua_CFunction func, const string& name);
        void runString(const string& str);
        void runFile(const string& path);
        vector<shared_ptr<Data>> call(const string& name, const vector<shared_ptr<Lua::Data>>& params);
    private:
        lua_State *state;
    };
}

#endif /* Lua_hpp */
