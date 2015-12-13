//
//  macros.h
//  FlansBasement
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

#endif /* macros_h */
