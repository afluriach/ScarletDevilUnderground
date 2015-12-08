//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

#include "LuaAPI.hpp"

namespace Lua{

const vector<string> Inst::luaIncludes = boost::assign::list_of
    ("util")
    ("class")
    ("math")
;

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }
    
    void runscript(string name){
        app->lua.runFile("scripts/"+name+".lua");
    }

//Lua API methods

    Inst::Inst()
    {
        state = luaL_newstate();
        //Load standard libraries
        luaL_openlibs(state);
        installApi();
        installWrappers();
        loadLibraries();
        
        if(catchLuaPanic)
            lua_atpanic(state, luaContextPanic);
    }
    
    Inst::~Inst()
    {
        lua_close(state);
    }
    
    void Inst::loadLibraries()
    {
        BOOST_FOREACH(auto s, luaIncludes){
            runFile("scripts/"+s+".lua");
        }
    }
    
    void Inst::installFunction(lua_CFunction func, const string& name)
    {
        lua_pushcfunction(state, func);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::setGlobal(LuaRef ref, const string& name)
    {
        ref.push(state);
        lua_setglobal(state, name.c_str());
    }
    
    void Inst::runString(const string& str)
    {        
        int error = luaL_dostring(state, str.c_str());
        
        if(error)
            log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
    {
        runString(loadTextFile(path));
    }
    
    void Inst::runObjectFile(const string& path)
    {
        luaL_loadfile(state, getRealPath(path).c_str());
        docall(state, 0, LUA_MULTRET);
    }

    bool Inst::globalExists(const string& name)
    {
        lua_getglobal(state, name.c_str());
        bool result = !lua_isnil(state, -1);
        lua_pop(state, 1);
        
        return result;
    }

    vector<LuaRef> Inst::callIfExists(const string& name, const vector<LuaRef>& params)
    {
        if(!globalExists(name))
            return vector<LuaRef>();
        
        return call(name, params);
    }

    vector<LuaRef> Inst::call(const string& name, const vector<LuaRef>& params)
    {
        int top = lua_gettop(state);
        
        lua_pushglobaltable(state);
        lua_getfield(state, -1, name.c_str());
        //Remove global table after pushing function to call
        lua_remove(state, -2);
        
        foreach(LuaRef r, params)
        {
            r.push(state);
        }
        
        try{
        
            int error = docall(state, params.size(), LUA_MULTRET);

            if(error)
            {
                log("Lua::Inst::call: %s, error: %s", name.c_str(), lua_tostring(state,-1));
                return vector<LuaRef>();
            }
            else
            {
                int nResults = lua_gettop(state) - top;
                vector<LuaRef> results;
                
                for(int i=0;i<nResults; ++i){
                    LuaRef ref(state);
                    ref.pop(state);
                    results.push_back(ref);
                }
                reverse(results.begin(), results.end());
                return results;
            }
        }
        catch(lua_runtime_error ex){
            log("Lua::Inst, %s: runtime exception %s.", name.c_str(), ex.what());
            return vector<LuaRef>();
        }
    };
    
    #define requireNonNil(ref, L, msg) \
        if(ref.isNil()){ \
            error(L, msg); \
            return 0; \
        }
    #define NARGS int nArgs = lua_gettop(L);

    char errorbuf[128];
    #define check_args(target)\
    if(nArgs != target){ \
        snprintf(errorbuf, 128, "%s: %d parameters required, %d found", crntFuncName, target, nArgs); \
        error(L, errorbuf); \
        return 1; \
    }
    
    #define get_gspace \
    GSpace* gspace = GScene::getSpace(); \
    if(!gspace){ \
        snprintf(errorbuf, 128, "%s: cannot use in current scene.", crntFuncName); \
        error(L, errorbuf); \
        return 1; \
    } \

    #define arg(name) \
    LuaRef name(L); \
    name.pop(L);
    
    //Create LuaRef from value stored in table under the same name.
    #define ref_from_table(table, name) LuaRef name = table[#name];
    
    #define func(name) \
    int name(lua_State* L) \
    { \
    const char* crntFuncName = #name;
    
    #define push_error(errorMsg) {\
    snprintf(errorbuf, 128, "%s: %s", crntFuncName, errorMsg); \
    error(L, errorbuf); \
    return 1; }

    #define c_str(string_expr) string(string_expr).c_str()
        
    //Lua API functions:
    
    func(luaContextPanic)

        throw lua_runtime_error("Lua Panic.");
        
        return 0;
    }
    
    func(log)
        //The first argument must be a string
        NARGS
        check_args(1);
        
        if(!lua_isstring(L,1)){
            push_error("first parameter must be string.")
        }

        //paramters are ones-based, arg count is at index 0;
        cocos2d::log("%s", lua_tostring(L,1));
        
        return 0;
    }

    int convertObj(lua_State* L)
    {
    const char* crntFuncName = "convert";
        NARGS
        check_args(2)

        arg(typeStr)
        arg(input)

        LuaRef result = convertObjectUserdata(input.cast<GObject*>(),typeStr.tostring(),L);

        result.push(L);
        return 1;
    }

    func(createObject)
        NARGS
        check_args(1)
        
        get_gspace
        
        arg(args);
        
        if(!args.isTable()){
            push_error("single table params required")
        }
        
        //Position and dimension are x,y vector
        ref_from_table(args,pos)
        ref_from_table(args,props)

        ref_from_table(args, name)
        ref_from_table(args, type)

        ref_from_table(args, width)
        ref_from_table(args, height)

        ref_from_table(args, radius)
        ref_from_table(args, vx)
        ref_from_table(args, vy)

        requireNonNil(name, L, "createObject: name required");
        requireNonNil(type, L, "createObject: type required");
        requireNonNil(pos,L, "createObject: pos required");

        Vec2 posV = getVec2FromTable(pos);

        map<string,string> m;
        
        if(!props.isNil() && props.isTable()){
            m = getStringMapFromTable(props, L);
        }
        m["name"] = name.tostring();
        m["type"] = type.tostring();
        
        m[lauArgTag] = "true";
        
        ValueMap objArg = GObject::makeValueMapArg(posV,m);
        
        //Dimensions, if included for dynamically sized objects, must be converted to pixel space to match existing interface. Otherwise provide 0 for dimension for objects that expect it to be there.
        
        objArg["width"] = width.isNumber() ? Value(getFloat(width)*App::pixelsPerTile) : Value(0.0);
        objArg["height"] = height.isNumber() ? Value(getFloat(height)*App::pixelsPerTile) : Value(0.0);

        if(radius.isNumber())
            objArg["radius"] = Value(getFloat(radius));
        
        if(vx.isNumber() && vy.isNumber()){
            objArg["vx"] = Value(getFloat(vx));
            objArg["vy"] = Value(getFloat(vy));
        }

        //Return reference to the created object.
        GObject* gobj = gspace->addObject(objArg);
        LuaRef obj(L, gobj->uuid);
        obj.push(L);

        return 1;
    }

    func(removeObject)
        NARGS
        check_args(1)
        arg(name)

        get_gspace
        gspace->removeObject(name.tostring());
        
        return 0;
    }
    
    func(runScene)
        NARGS
        check_args(1)
        arg(name)
        
        try{
            GScene::runScene(name.tostring());
        }
        catch(exception){
            push_error(c_str("runScene: " + name.tostring() + " not found"))
        }
        
        return 0;
    }
    
    func(castSpell)
        NARGS
        check_args(2)
        arg(spell)
        arg(caster)
        
        get_gspace
        
        GObject* obj = getObjectFromLuaData(caster);

        if(!obj){
            push_error(c_str("castSpell: " + caster.tostring() + " not found" ));
        }

        Spellcaster* sc = dynamic_cast<Spellcaster*>(obj);
        
        if(!sc){
            push_error(c_str("castSpell: " +caster.tostring() + " is not a Spellcaster."))
        }
        
        sc->cast(spell.tostring());
        return 0;
    }
    
    func(stopSpell)
        NARGS
        check_args(1)
        arg(caster)

        get_gspace
        
        GObject* obj = getObjectFromLuaData(caster);

        if(!obj){
            push_error(c_str("castSpell: " + caster.tostring() + " not found" ));
        }

        Spellcaster* sc = dynamic_cast<Spellcaster*>(obj);
        
        if(!sc){
            push_error(c_str("stopSpell: " + caster.tostring() + " is not a Spellcaster."))
        }
        
        sc->stop();
        return 0;
    }
    
    func(getObjectCount)
        NARGS
        check_args(0)
        
        get_gspace

        cocos2d::log("%d objects.", gspace->getObjectCount());
        
        return 0;
    }
    
    #define install(name) installFunction(name, #name)
    
    void Inst::installApi()
    {
        install(log);
        install(createObject);
        install(runScene);
        install(removeObject);
        install(castSpell);
        install(stopSpell);
        install(getObjectCount);
        installFunction(convertObj, "convert");
    }
}