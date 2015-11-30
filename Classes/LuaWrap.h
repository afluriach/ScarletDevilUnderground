//
//  LuaWrap.h
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#ifndef LuaWrap_h
#define LuaWrap_h

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

template<typename Ret, typename...Args>
int wrapFunc(const string& name, Ret (*func)(Args...), lua_State* L)
{
    list<LuaRef> luaArgs = Lua::getArgs(L);
    
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

#endif /* LuaWrap_h */
