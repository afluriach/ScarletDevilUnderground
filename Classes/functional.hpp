//
//  functional.hpp
//  Koumachika
//
//  Created by Toni on 6/27/18.
//
//

#ifndef functional_h
#define functional_h

#include "types.h"

//Bind this but not the explicit arguments.
template<typename T, typename A1>
inline function<void(A1)> bindMethod(void (T::* m)(A1), T* This)
{
    return bind(m, This, placeholders::_1);
}

template<typename T, typename A1, typename A2>
inline function<void(A1,A2)> bindMethod(void (T::* m)(A1,A2), T* This)
{
    return bind(m, This, placeholders::_1, placeholders::_2);
}

//call function using tuple
namespace detail
{
    template <typename Ret, typename F, typename Tuple, bool Done, int Total, int... N>
    struct call_impl
    {
        inline static Ret call(F f, Tuple && t)
        {
            return call_impl<Ret, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
        }
    };

    template <typename Ret, typename F, typename Tuple, int Total, int... N>
    struct call_impl<Ret, F, Tuple, true, Total, N...>
    {
        inline static Ret call(F f, Tuple && t)
        {
            return f(std::get<N>(std::forward<Tuple>(t))...);
        }
    };
}
template <typename Ret, typename F, typename Tuple>
inline Ret variadic_call(F f, Tuple && t)
{
    typedef typename std::decay<Tuple>::type ttype;
    return detail::call_impl<Ret, F, Tuple, 0 == std::tuple_size<ttype>::value, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
}

#define wrap_method(cls,method,This) wrapMethod<cls,&cls::method>(This)

//Wrapper to call a method of a derived type with a base this.
template<typename Class, void (Class::*Method)(void)>
inline function<void()> wrapMethod(Class* This)
{
    return [=]() -> void {
        mem_fn(Method)(This);
    };
}

template<typename T, typename... Args>
inline function<void(void)> generate_action(T* _this, void (T::*m)(Args...), Args ...args)
{
	return [=](void) -> void {
		invoke(m, _this, args...);
	};
}

template<typename T>
tuple<T> parse(const vector<string>& _v, int idx)
{
	return tuple<T>(boost::lexical_cast<T>(_v.at(idx)));
}

template <typename Crnt, typename Next, typename... Rest>
tuple<Crnt, Next, Rest...> parse(const vector<string>& _v, int idx)
{
	return tuple_cat(
		parse<Crnt>(_v, idx),
		parse<Next, Rest...>(_v, idx + 1)
	);
}

template<typename... T>
void callAdapter(function<void(T...)> static_method, vector<string> tokens)
{
	size_t n_args = tuple_size<tuple<T...>>::value;

	if (tokens.empty()) {
		log("callAdapter: empty input");
		return;
	}
	else if (tokens.size() - 1 != n_args) {
		log("callAdapter: %d arguments expected, %d found", n_args, tokens.size() - 1);
		return;
	}

	tuple<T...> args = parse<T...>(tokens, 1);
	variadic_call<void>(static_method, args);
}

template<typename... T>
InterfaceFunction makeInterfaceFunction(function<void(T...)> static_method)
{
	return [static_method](const vector<string>& tokens)->void {
		callAdapter(static_method, tokens);
	};
}

#endif /* functional_h */
