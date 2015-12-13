//
//  Lua.cpp
//  FlansBasement
//
//  Created by Toni on 11/21/15.
//
//

#include "Prefix.h"

//Copied from ltm.h.
//#define ttypename(x)	luaT_typenames_[(x) + 1]
//extern const char *const luaT_typenames_[LUA_NUMTAGS+2];

namespace Lua{

const vector<string> Inst::luaIncludes = boost::assign::list_of
    ("util")
    ("class")
    ("math")
    ("repl")
;

unordered_map<string, Inst*> Inst::instances;

vector<pair<string,string>> Inst::commandQueue;
mutex Inst::queueLock;

    //Raise Lua exception
    void error(lua_State* L, const string& msg)
    {
        lua_pushstring(L, msg.c_str());
        lua_error(L);
    }
    
    void runscript(string name){
        app->lua.runFile("scripts/"+name+".lua");
    }

    void requireType(lua_State* L, const string& name, LuaRef ref, int typeID)
    {
        if(!ref.isType(typeID)){
            error(
                L,
                StringUtils::format(
                    "%s: expected %s, got %s.",
                    name.c_str(),
                    ttypename(typeID),
                    ref.getTypeName()
                )
            );
        }
    }
    
    void replThreadMain(Inst * inst)
    {
        inst->call("open_repl", vector<LuaRef>());
    }

//Lua API methods

    Inst::Inst(const string& name) : name(name)
    {
        state = luaL_newstate();
        //Load standard libraries
        luaL_openlibs(state);
        installApi();
        installWrappers();
        loadLibraries();
        
        if(catchLuaPanic)
            lua_atpanic(state, luaContextPanic);

        cocos2d::log("Lua Inst created: %s.", name.c_str());
        
        if(instances.find(name) != instances.end())
            cocos2d::log("Lua Inst with duplicate name: %s.", name.c_str());
        instances[name] = this;
    }
    
    Inst::~Inst()
    {
        lua_close(state);
        instances.erase(name);
        cocos2d::log("Lua Inst closed: %s.", name.c_str());
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
    
    void Inst::addCommand(const string& target, const string& script)
    {
        queueLock.lock();
        commandQueue.push_back(pair<string,string>(target,script));
        queueLock.unlock();
    }
    
    void Inst::runCommands()
    {
        queueLock.lock();
        
        BOOST_FOREACH(auto command, commandQueue)
        {
            auto it = Inst::instances.find(command.first);
        
            if(it != Inst::instances.end()){
                it->second->runString(command.second);
            }
            else{
                cocos2d::log("runCommands: instance %s not found.", command.first.c_str());
            }
        }
        
        queueLock.unlock();
    }
    
    void Inst::runString(const string& str)
    {        
        int error = luaL_dostring(state, str.c_str());
        
        if(error)
            log("Lua error: %s", lua_tostring(state,-1));
    }
    
    void Inst::runFile(const string& path)
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

        requireType(L, "createObject: ", args, LUA_TTABLE);

        //pos (and dim and vel if included) are interpreted as Vec2
        ref_from_table(args,pos)
        ref_from_table(args, name)
        ref_from_table(args, type)

        requireType(L, "createObject: name", name,LUA_TSTRING);
        requireType(L, "createObject: type", type,LUA_TSTRING);
        requireType(L, "createObject: pos", pos,LUA_TTABLE);

        ValueMap objArg = GObject::makeValueMapArg(args);

        //Return reference to the created object.
        GObject* gobj = gspace->addObject(objArg);
        LuaRef obj = convert<GObject*>::convertToLua(gobj, L);
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

    func(open_repl)
        int nargs = lua_gettop(L);

        lua_State* target = nullptr;

        if(nargs == 1)
        {
            const char* name = lua_tostring(L, -1);
            auto it = Inst::instances.find(name);
            
            if(it != Inst::instances.end())
                target = it->second->state;
            else
                cocos2d::log("open_repl: instance %s not found.", name);
        }

        if(!target)
            target = app->lua.state;

        lua_pushboolean(target, 0);
        lua_setglobal(target, "exitREPL");

        doREPL(target);
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
//        install(open_repl);
        installFunction(convertObj, "convert");
        
        install(print_gc_stats);
    }
}