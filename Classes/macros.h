//
//  macros.h
//  Koumachika
//
//  Created by Toni on 11/25/15.
//
//

#ifndef macros_h
#define macros_h

#define foreach BOOST_FOREACH

#define for_irange(var,start,end) for(int var : boost::irange(static_cast<int>(start),static_cast<int>(end)))

//Convert to concrete container type.
//The list directly follows the list_of macro, and every element include the first
//is in parenthesis
#define list_of_typed(list,type) boost::assign::list_of list .convert_to_container<type>()

#define debug_log(s) log("%s, %d: %s", __FUNCTION__, __LINE__, s);

//Despite its name, it could map to another method of a different name
//with the same signature in the same class.
//Otherwise, superMethod identifier includes class name scope.
#define CallSuper(method, superMethod, ret, signature, args) \
inline ret method( signature ) \
{ \
    return superMethod(args); \
} \

#define no_op(method) inline void method() {}

#define expand_vector2(v)  ((v).x) , ((v).y) 

#define square(n) ( (n) * (n) )

#define set_float_arg(field, def) field = getFloatOrDefault(args, #field, def);
#define init_float_field(field,default) field(getFloatOrDefault(args, #field, default))

#define float_pi boost::math::constants::pi<float>()
#define float_2pi (float_pi * 2.0f)

#define enum_increment(cls,lval) lval = static_cast<cls>( static_cast<int>(lval) + 1 )

#define enum_foreach(cls,var_name,begin,end) for(cls var_name=cls::begin; var_name < cls::end; enum_increment(cls, var_name) )

#define delete_if(ptr) if(ptr) { delete ptr; ptr = nullptr; }

#define enum_bitwise_or(cls,a,b) static_cast<cls>(static_cast<unsigned int>(cls::a) | static_cast<unsigned int>(cls::b))

#define enum_bitwise_or3(cls,a,b,c) static_cast<cls>(static_cast<unsigned int>(cls::a) | static_cast<unsigned int>(cls::b) | static_cast<unsigned int>(cls::c))


#endif /* macros_h */
