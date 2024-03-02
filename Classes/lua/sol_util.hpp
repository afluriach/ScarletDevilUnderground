//
//  sol_util.hpp
//  Koumachika
//
//  Created by Toni on 3/11/21.
//
//

#ifndef sol_util_hpp
#define sol_util_hpp

namespace sol{

bool hasMethod(sol::table obj, const string& name);
void printErrorMessage(lua_State* state);

template<typename T>
inline bool getField(sol::table t, const string& clsName, T* dest)
{
	sol::object obj = t[clsName];

	if (obj) {
		*dest = obj.as<T>();
		return true;
	}
	return false;
}

template<typename... Args>
inline void runMethodIfAvailable(sol::table obj, const string& name, Args... args)
{
	if (!obj) return;
	auto f = obj[name];

    if (f.valid() && f.get_type() == type::function) {
		sol::function_result result = f(obj, args...);
  
        if(!result.valid()){
            printErrorMessage(obj.lua_state());
            throw runtime_error("lua error");
		}
	}
}

template<typename R, typename... Args>
inline R runMethodIfAvailableOrDefault(R _default, sol::table obj, const string& name, Args... args)
{
    if(obj && obj[name].valid())
        return runMethod<R,Args...>(obj, name, args...);
    else
        return _default;
}

template<typename R, typename... Args>
inline R runMethod(sol::table obj, const string& name, Args... args)
{
	if (!obj) throw error("run Method: nil object");
	auto f = obj[name];

    if (f.valid() && f.get_type() == type::function) {
		sol::function_result result = f(obj, args...);
  
        if(result.valid()){
            return result;
		}
        else{
            printErrorMessage(obj.lua_state());
			throw runtime_error("lua error");
		}
    }
}

template<typename T>
inline void init_script_object(T* _this, const object_params& params)
{
    auto objects = _this->space->scriptVM->_state["objects"];
    string name = _this->getScriptClsName();
    auto cls = objects[name];
    if (cls.valid()) {
		sol::function_result result = cls(_this, params);
		
		if(result.valid()){
			_this->_setScriptObj(result);
		}
		if(!result.valid()){
			printErrorMessage(_this->space->scriptVM->_state);
			throw runtime_error("lua error");
		}
    }
}

}

#endif
