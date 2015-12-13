//
//  LuaWrap.h
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#ifndef LuaWrap_h
#define LuaWrap_h

namespace Lua{

class Class
{
private:
    static unordered_map<string, Class> classes;
    static bool init;
    
    const string name;
    unordered_map<string, lua_CFunction> methods;
public:
    static void makeClasses();
    
    inline static void installClasses(lua_State* L)
    {
        if(!init){
            makeClasses();
            init = true;
        }
    
        foreach(auto classEntry, classes){
            classEntry.second.installClass(L);
        }
    }
    
    inline static void makeClass(string name)
    {
        classes.insert(make_pair(name, Class(name)));
    }

    inline static void addMethod(string clsName, string methodName, lua_CFunction wrapper)
    {
        classes.at(clsName).methods.insert(make_pair(methodName, wrapper));
    }

    inline Class(const string& name) : name(name)
    {
    }

    inline Class(const Class& other) noexcept :
    name(other.name),
    methods(other.methods)
    {
    }
    
    inline Class(Class&& c) :
    name(move(c.name)),
    methods(move(c.methods))
    {
    }
    
    inline void installClass(lua_State* L)
    {
        makeMetatable(L);
        
        //Add all wrapped methods
        
        //Get metatable
        lua_getglobal(L, name.c_str());

        foreach(auto methodEntry, methods){
            string name = methodEntry.first;
            lua_CFunction wrapper = methodEntry.second;
            
            //Add method
            
            //for table assign, value is top of stack and key is second.
            lua_pushstring(L, name.c_str());
            lua_pushcfunction(L, wrapper);
            
            lua_rawset(L, -3);
        }
    }
    
    inline void makeMetatable(lua_State* L)
    {
        //check if it already exists, which shouldn't happen unless the
        //create constructor was inappopriately invoked.

        lua_newtable(L);
        lua_setglobal(L,name.c_str());
        
        //Set __index
        lua_getglobal(L,name.c_str());
        lua_pushstring(L, "__index");
        lua_getglobal(L,name.c_str());
        lua_rawset(L, -3);
    }
    
    static inline void setMetatable(const string& name, lua_State* L)
    {
        if(!lua_istable(L, -1))
            throw runtime_error("setMetatable: top of stack must table.");
        
        lua_getglobal(L, name.c_str());
        
        if(lua_isnil(L, -1))
            throw runtime_error(name + " metatable not found.");
        
        lua_setmetatable(L, -2);
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
        list<LuaRef> luaArgs = getArgs(L);

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
        list<LuaRef> luaArgs = getArgs(L);

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

template<typename Ret, typename C, typename...Args>
struct wrapMethod{
    static int wrap(const string& name, Ret (C::*func)(Args...), lua_State* L)
    {
        tuple<C*, Args...> args = cargs<C*,Args...>::getCArgs(L, name);
        
        C* This = get<0>(args);
        if(!This){
            log("wrapMethod %s: invalid this for first argument", name.c_str());
            return 0;
        }

        //Return type (first template parameter) cannot be inferred.
        Ret r = variadic_call<Ret>(mem_fn(func), args);
        LuaRef luaRet = convert<Ret>::convertToLua(r, L);
        luaRet.push(L);
        return 1;
    }
};

template<typename C, typename...Args>
struct wrapMethod<void,C,Args...>{
    static int wrap(const string& name, void (C::*func)(Args...), lua_State* L)
    {
        tuple<C*, Args...> args = cargs<C*,Args...>::getCArgs(L, name);

        C* This = get<0>(args);
        if(!This){
            log("wrapMethod %s: invalid this for first argument", name.c_str());
            return 0;
        }

        //Return type (first template parameter) cannot be inferred.
        variadic_call<void>(mem_fn(func), args);
        
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

template<typename C, typename Ret, typename...Args>
int methodWrapper(const string& name, Ret (C::*func)(Args...), lua_State* L)
{
    return wrapMethod<Ret,C,Args...>::wrap(name, func, L);
}


};//namespace

#endif /* LuaWrap_h */
