//
//  LuaCWrappers.cpp
//  FlansBasement
//
//  Created by Toni on 11/29/15.
//
//

#include "Prefix.h"
#include "LuaWrap.h"

void setVel(string name, float x, float y)
{
    GObject* obj = GScene::getSpace()->getObject(name);
    
    if(obj)
        obj->body->setVel(SpaceVect(x,y));
    else
        throw runtime_error("setVel: " + name + " not found");
}

//just for testing
void sv(float v, float x, float y)
{
    log("%f, %f, %f", v, x,y);
}

#define make_wrapper(name) \
int name ## _wrapper(lua_State* L) \
{ \
    try{ \
        return wrapFunc(#name, name, L);\
    }catch(runtime_error err){ \
        Lua::error(L, err.what()); \
        return 1; \
    } \
}

#define install_wrapper(name) installFunction(name ## _wrapper, #name);

make_wrapper(setVel)
make_wrapper(sv)

namespace Lua
{
    void Inst::installWrappers()
    {
        install_wrapper(setVel)
        install_wrapper(sv)
    }

}