//
//  macros.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef macros_h
#define macros_h

#define to_int(x) static_cast<int>(x)
#define for_irange(var,start,end) for(int var : boost::irange(to_int(start),to_int(end)))

#define debug_log(s) log("%s, %d: %s", __FUNCTION__, __LINE__, s);

#define no_op(method) inline void method() {}

#define expand_vector2(v)  ((v).x) , ((v).y) 

#define square(n) ( (n) * (n) )

#define set_float_arg(field, def) field = getFloatOrDefault(args, #field, def);
#define init_float_field(field,default) field(getFloatOrDefault(args, #field, default))

#define float_pi boost::math::constants::pi<cp::Float>()
#define float_2pi (float_pi * 2.0f)

#define delete_if(ptr) if(ptr) { delete ptr; ptr = nullptr; }

#endif /* macros_h */
