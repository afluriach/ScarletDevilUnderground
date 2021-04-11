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
inline void runtMethodIfAvailable(sol::table obj, const string& name, Args... args)
{
	if (!obj) return;
	auto f = obj[name];

    if (f.valid() && f.get_type() == type::function) {
		f(args...);
	}
}

}

#endif
