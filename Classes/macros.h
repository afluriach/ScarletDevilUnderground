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
//Convert to concrete container type.
//The list directly follows the list_of macro, and every element include the first
//is in parenthesis
#define list_of_typed(list,type) boost::assign::list_of list .convert_to_container<type>()

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

//Requires an appropriately defined wrap method (currently only GScene and GObject support this)
#define wrap_method(cls,method,This) wrapMethod<cls,&cls::method>(This)

#define enum_increment(cls,lval) lval = static_cast<cls>( static_cast<int>(lval) + 1 )

#define enum_foreach(cls,var_name,begin,end) for(cls var_name=cls::begin; var_name < cls::end; enum_increment(cls, var_name) )

#endif /* macros_h */
