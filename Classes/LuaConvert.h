//
//  LuaConvert.h
//  Koumachika
//
//  Created by Toni on 12/5/15.
//
//

#ifndef LuaConvert_h
#define LuaConvert_h

class GObject;

namespace Lua{

bool tableIsVec2(LuaRef t);
IntVec2 getInt2FromTable(LuaRef t);
Vec2 getVec2FromTable(LuaRef t);
Value cocosValueFromLua(LuaRef);
function<void()> makeFunctorFromLuaFunction(LuaRef ref);
list<LuaRef> getArgs(lua_State* L);
unordered_map<string,string> getStringMapFromTable(LuaRef table, lua_State* state);
float getFloat(LuaRef r);
LuaRef convertObjectUserdata(GObject* o, const string& typeStr, lua_State* L);
GObject* getObjectFromLuaData(LuaRef ref);

void check_integer_value(LuaRef ref);

template<typename K, typename V>
unordered_map<K,V> getMapFromTable(LuaRef table)
{
    unordered_map<K,V> result;
    
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


template<typename T>
void check_numeric_range(LuaRef ref)
{
    double d = ref.cast<double>();
    
    //allow subnormal float to be rounded.
    double dmax = static_cast<double>(numeric_limits<T>::max());
    if(d > dmax){
        throw lua_type_error(StringUtils::format("Lua number is outside of %s range.", typeid(T).name()));
    }
}

template<typename T>
void check_signed(LuaRef ref)
{
    double d = ref.cast<double>();
    
    if(!is_signed<T>::value && d < 0)
        throw lua_type_error("Attempt to cast negative Lua number to unsigned.");
}

//Base
template<typename T, typename = void>
struct convert{
};

//General case for all numeric types.
template<typename T>
struct convert<T, typename enable_if<is_arithmetic<T>::value>::type>{
    inline static T convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        if(typeid(T) != typeid(float) && typeid(T) != typeid(double))
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

template<typename T>
struct convert<T, typename enable_if<is_enum<T>::value>::type>{
    inline static T convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return static_cast<T>(ref.cast<int>());
    }
    inline static LuaRef convertToLua(const T& t, lua_State* L)
    {
        return LuaRef(L, static_cast<int>(t));
    }
};

template<>
struct convert<bool>{
    inline static bool convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return ref.cast<bool>();
    }
    inline static LuaRef convertToLua(const bool& b, lua_State* L)
    {
        return LuaRef(L, b);
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

template<>
struct convert<SpaceVect>{
    inline static SpaceVect convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        Vec2 v = getVec2FromTable(ref);
        return SpaceVect(v.x, v.y);
    }
    inline static LuaRef convertToLua(const SpaceVect& v, lua_State* L)
    {
        LuaRef table(L);
        table = newTable(L);
        
        table["x"] = v.x;
        table["y"] = v.y;
        
        return table;
    }
};

template<>
struct convert<IntVec2>{
    inline static IntVec2 convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return getInt2FromTable(ref);
    }
    inline static LuaRef convertToLua(const IntVec2& v, lua_State* L)
    {
        LuaRef table(L);
        table = newTable(L);
        
        //Even though IntVec2 is actually a pair<int,int>, we will maintain vector semantics
        //when passing to Lua.
        table["x"] = v.first;
        table["y"] = v.second;
        
        return table;
    }
};

template<>
struct convert<ValueMap>{
    static ValueMap convertFromLua(const string& name, int argNum, LuaRef ref);
    static LuaRef convertToLua(ValueMap obj, lua_State* L);

};

template<>
struct convert<GObject*>{
    static GObject* convertFromLua(const string& name, int argNum, LuaRef ref);
    static LuaRef convertToLua(GObject* obj, lua_State* L);
};

template<typename C>
struct convert<C*>{
    inline static C* convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        return ref.cast<C*>();
    }
    inline static LuaRef convertToLua(C* c, lua_State* L)
    {
        if(!c) return LuaRef(L);
        
        return LuaRef(L, c);
    }
};

template<>
struct convert<function<void()>>{
    inline static function<void()> convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        if(!ref.isFunction())
            throw lua_type_error(StringUtils::format("%s, expected a function for arg %d", name.c_str(), argNum));
    
        return makeFunctorFromLuaFunction(ref);
    }
//    inline static LuaRef convertToLua(const function<void>& t, lua_State* L)
//    {
//        throw lua_type_error("Return C functor not supported.");
//    }
};

template<typename T>
struct convert<vector<T>>{

    inline static vector<T> convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        vector<T> result;
        
        for(int i=1; !ref[i].isNil(); ++i)
        {
            result.push_back(convert<T>::convertFromLua(name, argNum, ref[i]));
        }
        return result;
    }
    inline static LuaRef convertToLua(const vector<T>& v, lua_State* L)
    {
        LuaRef table(L);
        table = newTable(L);
        
        for(int i = 1; i <= v.size(); ++i)
        {
            table[i] = convert<T>::convertToLua(v[i-1], L);
        }
        
        return table;
    }
};

template<typename T, typename U>
struct convert<pair<T,U>>{

    inline static pair<T,U> convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        pair<T,U> result;

        result.first = convert<T>::convertFromLua(name, argNum, ref[1]);
        result.first = convert<T>::convertFromLua(name, argNum, ref[2]);
        
        return result;
    }
    inline static LuaRef convertToLua(const pair<T,U>& p, lua_State* L)
    {
        LuaRef table(L);
        table = newTable(L);
        
        table[1] = convert<T>::convertToLua(p.first, L);
        table[2] = convert<T>::convertToLua(p.second, L);
        
        return table;
    }
};

template<typename K,typename V>
struct convert<unordered_map<K,V>>{
    inline static unordered_map<K,V> convertFromLua(const string& name, int argNum, LuaRef ref)
    {
        if(not ref.isTable())
            throw lua_type_error(" is not a table");
        
        return getMapFromTable<K,V>(ref);
    }
    inline static LuaRef convertToLua(const unordered_map<K,V>& m, lua_State* L)
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

}

#endif /* LuaConvert_h */
