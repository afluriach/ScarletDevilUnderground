//
//  LuaError.h
//  Koumachika
//
//  Created by Toni on 12/5/15.
//
//

#ifndef LuaError_h
#define LuaError_h

namespace Lua{

    class lua_runtime_error : public runtime_error
    {
    public:
        inline lua_runtime_error(string what) : runtime_error(what) {}
        inline lua_runtime_error(const string& name, int arg, const string& msg)
        : lua_runtime_error(StringUtils::format("%s, argument %d: %s", name.c_str(), arg, msg.c_str())){}

    };
    
    class lua_type_error : public lua_runtime_error
    {
    public:
        inline lua_type_error(string what) : lua_runtime_error(what) {}
        inline lua_type_error(const string& name, int arg, const string& msg)
        : lua_runtime_error(name, arg, msg){}

    };

}

#endif /* LuaError_h */
