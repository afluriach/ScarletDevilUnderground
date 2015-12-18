//
//  LuaCWrappers.cpp
//  Koumachika
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "LuaWrap.h"

namespace Lua{

unordered_map<string, Class> Class::classes;
bool Class::init = false;

void setpos(string name, float x, float y)
{
    GObject* obj = GScene::getSpace()->getObject(name);
    
    if(obj)
        obj->body->setPos(SpaceVect(x,y));
    else
        throw runtime_error("setpos: " + name + " not found");
}

void setvel(string name, float x, float y)
{
    GObject* obj = GScene::getSpace()->getObject(name);
    
    if(obj)
        obj->body->setVel(SpaceVect(x,y));
    else
        throw runtime_error("setvel: " + name + " not found");
}

//just for testing
void sv(float v, float x, unsigned int y)
{
    log("%f, %f, %ud", v, x,y);
}

int getObjCount()
{
    GSpace* space = GScene::getSpace();
    
    if(!space) return -1;
    return space->getObjectCount();
}

unordered_map<int, string> getUUIDNameMap()
{
    GSpace* space = GScene::getSpace();
    
    if(!space) return unordered_map<int,string>();
    
    return space->getUUIDNameMap();
}

void printMap(unordered_map<string,string> m)
{
    for(auto it = m.begin(); it != m.end(); ++it){
        log("%s : %s", it->first.c_str(), it->second.c_str());
    }
}

void addUpdate(function<void()> f, int order)
{
    //Wrapper with a dummy GScene pointer that isn't used.
    GScene::crntScene->multiUpdate.insertWithOrder(
        [=](GScene* g) -> void{
            f();
        },
        order
    );
}

void setscreenscale(float f)
{
    app->screenscale = f;
}

GObject* getObjByName(string name)
{
    GSpace* space = GScene::getSpace();
    
    if(!space) lua_runtime_error("Cannot access objects in this scene.");
    
    return space->getObject(name);
}

vector<string> getObjectNames()
{
    GSpace* space = GScene::getSpace();
    
    if(!space) lua_runtime_error("Cannot access objects in this scene.");

    return space->getObjectNames();
}

//The Lua conversion template will convert the wrapper object to a valid pointer or null.
bool isValidObject(GObject* object)
{
    return object != nullptr;
}

void setPlayerHealth(int val)
{
    GSpace* space = GScene::getSpace();
    
    if(!space) lua_runtime_error("Cannot access objects in this scene.");
    
    Player* p = space->getObject<Player>("player");
    
    if(!p)
        lua_runtime_error("Player is not available.");
    
    if(val <= 0 || val > Player::maxHealth)
        lua_runtime_error("setPlayerHealth: value outside valid range.");
    
    p->setHealth(val);
}

void setPaused(bool val)
{
    GScene::crntScene->setPaused(val);
}

void dostring_in_inst(string script, string inst_name)
{
    Inst::addCommand(inst_name, script);
}

void castSpellWithArgs(string casterName, string spell, ValueMap args)
{
    GSpace* space = GScene::getSpace();
    if(!space) lua_runtime_error("Cannot access objects in this scene.");
    Spellcaster* caster = space->getObject<Spellcaster>(casterName);

    if(!caster){
        throw lua_runtime_error("castSpell: Spellcaster " + casterName + " not found");
    }

    caster->cast(spell, args);
}
    
void castSpell(string casterName, string spell)
{
    castSpellWithArgs(casterName, spell, ValueMap());
}

void stopSpell(string casterName)
{
    GSpace* space = GScene::getSpace();
    if(!space) lua_runtime_error("Cannot access objects in this scene.");
    Spellcaster* caster = space->getObject<Spellcaster>(casterName);

    if(!caster){
        throw lua_runtime_error("castSpell: Spellcaster " + casterName + " not found");
    }

    caster->stop();
}

bool isObstacle(IntVec2 v)
{
    GSpace* space = GScene::getSpace();
    if(!space) lua_runtime_error("Cannot access objects in this scene.");

    return space->isObstacle(v);
}

///////////////////////////////////////////////////////////////////

#define make_wrapper(name) \
int name ## _wrapper(lua_State* L) \
{ \
    try{ \
        return wrapper(#name, name, L);\
    }catch(runtime_error err){ \
        Lua::error(L, err.what()); \
        return 1; \
    } \
}

#define make_method_wrapper(cls, name) \
int name ## _wrapper(lua_State* L) \
{ \
    try{ \
        return methodWrapper(#cls "::" #name, &cls::name, L);\
    }catch(runtime_error err){ \
        Lua::error(L, err.what()); \
        return 1; \
    } \
}

#define install_wrapper(name) installFunction(name ## _wrapper, #name);
#define install_method_wrapper(cls,name) installFunction(name ## _wrapper, #cls "_" #name);
#define add_method(cls,name) Class::addMethod(#cls, #name, name ## _wrapper);


make_wrapper(setpos)
make_wrapper(setvel)
make_wrapper(sv)
make_wrapper(getObjCount)
make_wrapper(getUUIDNameMap)
make_wrapper(printMap)
make_wrapper(addUpdate)
make_wrapper(setscreenscale)
make_wrapper(getObjByName)
make_wrapper(getObjectNames)
make_wrapper(isValidObject)
make_wrapper(runscript)
make_wrapper(setPlayerHealth)
make_wrapper(setPaused)
make_wrapper(dostring_in_inst)
make_wrapper(castSpell)
make_wrapper(castSpellWithArgs)
make_wrapper(stopSpell)
make_wrapper(isObstacle)

//Utility functions not specifically created for the scripting API
make_wrapper(toDirection)

make_method_wrapper(GObject,getPos)
make_method_wrapper(GObject,setPos)
make_method_wrapper(GObject,getVel)
make_method_wrapper(GObject,setVel)
make_method_wrapper(GObject, setDirection)
make_method_wrapper(GObject,getUUID)
make_method_wrapper(GObject,getName)
make_method_wrapper(Spellcaster,stop)

void Class::makeClasses()
{
    Class::makeClass("GObject");
    Class::makeClass("Spellcaster");
    
    add_method(GObject,getPos)
    add_method(GObject,setPos)
    add_method(GObject,getVel)
    add_method(GObject,setVel)
    add_method(GObject, setDirection)
    add_method(GObject,getUUID)
    add_method(GObject,getName)
    add_method(Spellcaster,stop)
}

void Inst::installWrappers()
{
    install_wrapper(setpos)
    install_wrapper(setvel)
    install_wrapper(sv)
    install_wrapper(getObjCount)
    install_wrapper(getUUIDNameMap)
    install_wrapper(printMap)
    install_wrapper(addUpdate)
    install_wrapper(setscreenscale)
    install_wrapper(getObjByName)
    install_wrapper(getObjectNames)

    install_wrapper(isValidObject)
    install_wrapper(runscript)
    install_wrapper(setPlayerHealth)
    install_wrapper(setPaused)
    install_wrapper(dostring_in_inst)
    install_wrapper(castSpell)
    install_wrapper(castSpellWithArgs)
    install_wrapper(stopSpell)
    install_wrapper(isObstacle)
    
    //Utility functions not specifically created for the scripting API
    install_wrapper(toDirection)
    
    Class::installClasses(state);
}

}