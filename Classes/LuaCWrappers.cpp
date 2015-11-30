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

int getObjCount()
{
    GSpace* space = GScene::getSpace();
    
    if(!space) return -1;
    return space->getObjectCount();
}

//The wrap template (which contains wrapFunc) can no longer be matched automatically from the type of the function pointer
//now that it has specialization. Now a template wrapper function has to declared to explicitly define wraps<> template arguments.
template<typename Ret, typename...Args>
int wrapper(const string& name, Ret (*func)(Args...), lua_State* L)
{
    return wrap<Ret,Args...>::wrapFunc(name, func, L);
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

make_wrapper(setVel)
make_wrapper(sv)
make_wrapper(getObjCount)

namespace Lua
{
    void Inst::installWrappers()
    {
        install_wrapper(setVel)
        install_wrapper(sv)
        install_wrapper(getObjCount)
    }

}