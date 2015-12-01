//
//  LuaWrap.h
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#ifndef LuaWrap_h
#define LuaWrap_h

#include "LuaAPI.hpp"

#define _lua_type_error(msg) Lua::lua_type_error(name, argNum,msg)

template<typename T>
void check_numeric_range(LuaRef ref)
{
    double d = ref.cast<double>();
    
    if(d > numeric_limits<T>::max()
    || d < numeric_limits<T>::min()
    ){
        throw Lua::lua_type_error(StringUtils::format("Lua number is outside of %s range.", typeid(T).name()));
    }
}

void check_integer_value(LuaRef ref)
{
    double d = ref.cast<double>();
    if(floor(d) != d)
        throw Lua::lua_type_error("Attempt to cast non-integer Lua number to integer.");
}

template<typename T>
void check_signed(LuaRef ref)
{
    double d = ref.cast<double>();
    
    if(!is_signed<T>::value && d < 0)
        throw Lua::lua_type_error("Attempt to cast negative Lua number to unsigned.");
}

//General case for all numeric types.
template<typename T>
struct convert{
    inline static T convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        check_integer_value(ref);
        check_signed<T>(ref);
        check_numeric_range<T>(ref);
    
        return ref.cast<T>();
    }
    inline static LuaRef convertToLua(const T& t, lua_State* L)
    {
        return LuaRef(L, t);
    }
};

template<>
struct convert<string>{
    inline static string convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return ref.tostring();
    }
    inline static LuaRef convertToLua(const string& t, lua_State* L)
    {
        return LuaRef(L, t);
    }
};


template<typename K,typename V>
struct convert<map<K,V>>{
    inline static map<K,V> convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        if(not ref.isTable())
            throw runtime_error(_lua_type_error(" is not a table"));
        
        return Lua::getMapFromTable<K,V>(ref);
    }
    inline static LuaRef convertToLua(const map<K,V>& m, lua_State* L)
    {
        LuaRef table(L);
        table = newTable(L);
        
        for(auto it = m.begin(); it != m.end(); ++it){
            K key = it->first;
            V value = it->second;
            
            table[key] = value;
        }
        
        return table;
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
