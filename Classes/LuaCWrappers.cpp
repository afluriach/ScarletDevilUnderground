//
//  LuaCWrappers.cpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "LuaWrap.h"

namespace Lua{

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

map<int, string> getUUIDNameMap()
{
    GSpace* space = GScene::getSpace();
    
    if(!space) return map<int,string>();
    
    return space->getUUIDNameMap();
}

void printMap(map<string,string> m)
{
    for(auto it = m.begin(); it != m.end(); ++it){
        log("%s : %s", it->first.c_str(), it->second.c_str());
    }
}

void addUpdate(function<void()> f, int order)
{
    GScene::crntScene->multiUpdate.insertWithOrder(f, order);
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


make_wrapper(setpos)
make_wrapper(setvel)
make_wrapper(sv)
make_wrapper(getObjCount)
make_wrapper(getUUIDNameMap)
make_wrapper(printMap)
make_wrapper(addUpdate)
make_wrapper(setscreenscale)
make_wrapper(getObjByName)
make_wrapper(runscript)
make_wrapper(setPlayerHealth)

make_method_wrapper(GObject,getPos)
make_method_wrapper(GObject,setPos)
make_method_wrapper(GObject,setVel)
make_method_wrapper(GObject,getUUID)
make_method_wrapper(Spellcaster,castByName)
make_method_wrapper(Spellcaster,stop)

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
    install_wrapper(runscript)
    install_wrapper(setPlayerHealth)
    
//    getGlobalNamespace(state)
//        .beginClass<GObject>("GObject")
//            .addFunction("init", &GObject::init)
//            .addFunction("applyImpulse", &GObject::applyImpulse)
//        .endClass()
//        .deriveClass<Spellcaster, GObject>("Spellcaster")
//            .addFunction("cast", &Spellcaster::castByName)
//            .addFunction("stop", &Spellcaster::stop)
//        .endClass();
    
    install_method_wrapper(GObject,getPos)
    install_method_wrapper(GObject,setPos)
    install_method_wrapper(GObject,setVel)
    install_method_wrapper(GObject,getUUID)
    install_method_wrapper(Spellcaster, castByName)
    install_method_wrapper(Spellcaster,stop)

}

}