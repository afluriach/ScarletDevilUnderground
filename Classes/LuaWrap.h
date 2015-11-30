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
    static T convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return ref.cast<T>();
    }
    static LuaRef convertToLua(const T& t, lua_State* L)
    {
        return LuaRef(L, t);
    }
};

//conversion helper
template<typename T>
T convertLuaArg(const string& name, int argNum, LuaRef ref)
{
    return convert<T>::convertFromLua(name, argNum, ref);
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

//template<typename Ret, typename...Args>
//struct wrap
//{
//};

template<typename... Args>
struct cargs{
    static tuple <Args...> getCArgs(lua_State* L, const string& name)
    {
        list<LuaRef> luaArgs = Lua::getArgs(L);

        size_t c_arity = tuple_size<tuple<Args...>>::value;
        
        if(luaArgs.size() > c_arity){
            log("%s wrapper: ignoring extra parameters, %zu expected.", name.c_str(), c_arity);
        }
        else if(luaArgs.size() < c_arity){
            throw runtime_error(name + ": missing parameters, " + boost::lexical_cast<string>(c_arity) + " expected.");
        }
        tuple<Args...> args = parse<Args...>(name, luaArgs);

        return args;
    }
};

template<>
struct cargs<>{
    static tuple <> getCArgs(lua_State* L, const string& name)
    {
        list<LuaRef> luaArgs = Lua::getArgs(L);

        size_t c_arity = 0;
        
        if(luaArgs.size() > c_arity){
            log("%s wrapper: ignoring extra parameters, 0 expected.", name.c_str());
        }

        return tuple<>();
    }
};

//Wrap takes Lua arguments off the stack and converts them to a C tuple representing the required
//signature to variadic_call the wrapped C function. For now it uses the default LuaRef template
//casting, which will throw a Lua error if any parameter is not convertible.
//
//Wrap must be specialized to account for a void return type.
//If there is a return type, the LuaRef conversion constructor is used.

template<typename Ret, typename...Args>
struct wrap{
    static int wrapFunc(const string& name, Ret (*func)(Args...), lua_State* L)
    {
        tuple<Args...> args = cargs<Args...>::getCArgs(L, name);
        
        //Return type (first template parameter) cannot be inferred.
        Ret r = variadic_call<Ret>(func, args);
        LuaRef luaRet = convert<Ret>::convertToLua(r, L);
        luaRet.push(L);
        return 1;
    }
};

template<typename...Args>
struct wrap<void, Args...>{
    static int wrapFunc(const string& name, void (*func)(Args...), lua_State* L)
    {
        tuple<Args...> args = cargs<Args...>::getCArgs(L, name);

        //Return type (first template parameter) cannot be inferred.
        variadic_call<void>(func, args);
        return 0;
    }
};

//The top level function that can be called to perform the wrapping.
//
//The wrap template (which contains wrapFunc) can no longer be matched automatically from the type of the function pointer
//now that it has specialization. Now a template wrapper function has to declared to explicitly define wraps<> template arguments.
template<typename Ret, typename...Args>
int wrapper(const string& name, Ret (*func)(Args...), lua_State* L)
{
    return wrap<Ret,Args...>::wrapFunc(name, func, L);
}

#endif /* LuaWrap_h */
