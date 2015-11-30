//
//  LuaCWrappers.cpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"

#define type_is(target) typeid(T) == typeid(target)

//May be specialized, but use LuaRef cast by default.
template<typename T>
struct convert{
    static T convertLua(const string& name, int argNum, LuaRef ref)
    {
        return ref.cast<T>();
    }
};

//conversion helper
template<typename T>
T convertLuaArg(const string& name, int argNum, LuaRef ref)
{
    return convert<T>::convertLua(name, argNum, ref);
}

//build tuple by processing untyped input
template <typename T> T read(const string& name, int argNum, list<LuaRef>& refs)
{
    LuaRef ref = refs.front();
    refs.pop_front();
    return convertLuaArg<T>(name, argNum, ref);
}

template <typename T>
std::tuple<T> parse(const string& name, int argNum, list<LuaRef>& refs)
{
    T t = read<T>(name, argNum, refs);
    return tuple<T>(t);
}

template <typename Crnt, typename Next, typename... Rest>
std::tuple<Crnt, Next, Rest...> parse(const string& name, int argNum, list<LuaRef>& refs)
{
    Crnt c = read<Crnt>(name, argNum, refs);
    
    return tuple_cat(
        tuple<Crnt>(c),
        parse<Next, Rest...>(name, argNum+1, refs)
    );
}

template <typename... Args>
std::tuple<Args...> parse(const string& name, list<LuaRef> refs)
{
  return parse<Args...>(name, 1, refs);
}

list<LuaRef> getArgs(lua_State* L)
{
    int nArgs = lua_gettop(L);
    list<LuaRef> result;
    
    for(int i=0;i<nArgs; ++i){
        LuaRef ref(L);
        ref.pop(L);

        //For some reason this first log statement always prints the first parameter.
        //log("arg %d: %s", i+1, ref.tostring().c_str());
        result.push_back(ref);
    }
    result.reverse();

    //But here it shows them stored correctly.
//    foreach(LuaRef ref, result){
//        log("arg %s", ref.tostring().c_str());
//    }
    
    return result;
}

void setVel(string name, float x, float y)
{
    log("%s, %f, %f", name.c_str(), x,y);
    GObject* obj = GScene::getSpace()->getObject(name);
    obj->body->setVel(SpaceVect(x,y));
}

//just for testing
void sv(float v, float x, float y)
{
    log("%f, %f, %f", v, x,y);
}

template<typename Ret, typename...Args>
int wrapFunc(const string& name, Ret (*func)(Args...), lua_State* L)
{
    list<LuaRef> luaArgs = getArgs(L);
    
    size_t c_arity = tuple_size<tuple<Args...>>::value;
    
    if(luaArgs.size() > c_arity){
        log("%s wrapper: ignoring extra parameters, %zu expected.", name.c_str(), c_arity);
    }
    else if(luaArgs.size() < c_arity){
        log("%s wrapper: missing parameters, %zu expected.", name.c_str(), c_arity);
        return 1;
    }
    
    tuple<Args...> args = parse<Args...>(name, luaArgs);
    call(func, args);
    
    return 0;
}

int setVel_wrapper(lua_State* L)
{
    return wrapFunc("setVel", setVel, L);
}

int sv_wrapper(lua_State* L)
{
    return wrapFunc("sv", sv, L);
}

namespace Lua
{
    void Inst::installWrappers()
    {
        installFunction(setVel_wrapper, "setVel");
        installFunction(sv_wrapper, "sv");
    }

}