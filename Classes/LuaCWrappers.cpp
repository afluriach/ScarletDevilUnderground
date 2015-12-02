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

#define install_wrapper(name) installFunction(name ## _wrapper, #name);

make_wrapper(setpos)
make_wrapper(setvel)
make_wrapper(sv)
make_wrapper(getObjCount)
make_wrapper(getUUIDNameMap)
make_wrapper(printMap)
make_wrapper(addUpdate)
make_wrapper(setscreenscale)

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
}

}