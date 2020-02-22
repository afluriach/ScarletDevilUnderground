//
//  macros.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef macros_h
#define macros_h

#define to_char(x) static_cast<char>(x)
#define to_uchar(x) static_cast<unsigned char>(x)
#define to_int(x) static_cast<int>(x)
#define to_uint(x) static_cast<unsigned int>(x)
#define to_float(x) static_cast<float>(x)
#define to_gobject(x) static_cast<GObject*>(x)
#define bool_int(x) static_cast<int>(static_cast<bool>(x))
#define to_bool(x) static_cast<bool>(x)

#define for_irange(var,start,end) for(int var : boost::irange(to_int(start),to_int(end)))

#define no_op(method) inline void method() {}

#define expand_vector2(v)  ((v).x) , ((v).y) 

#define square_expr(n) ( (n) * (n) )

#define set_float_arg(field, def) field = getFloatOrDefault(args, #field, def);
#define init_float_field(field,default) field(getFloatOrDefault(args, #field, default))

#define float_pi boost::math::constants::pi<SpaceFloat>()
#define float_2pi (float_pi * 2.0f)

#define delete_if(ptr) if(ptr) { delete ptr; ptr = nullptr; }

#define getter(type,field) inline type get_ ## field() const { return field;}
#define setter(type,field) inline void set_ ## field(type v) { field = v;}

#define make_static_member_detector(x) \
template<typename T> \
struct has_##x \
{ \
private: \
	template<typename U> \
	static auto test(int) -> decltype(U::x, true_type()); \
\
	template<typename> \
	static false_type test(...); \
public: \
	static constexpr bool value = is_same<decltype(test<T>(0)), true_type>::value; \
}; \

#endif /* macros_h */
