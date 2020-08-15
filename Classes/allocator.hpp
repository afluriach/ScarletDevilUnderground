//
//  allocator.hpp
//  Koumachika
//
//  Created by Toni on 8/14/20.
//
//

#ifndef allocator_hpp
#define allocator_hpp

template<typename T>
using local_allocator = boost::fast_pool_allocator<
    T,
    boost::default_user_allocator_new_delete,
    boost::details::pool::null_mutex
>;

template<typename T, typename... Params>
T* allocator_new(Params... params)
{
    T* obj = local_allocator<T>::allocate();
    new (obj) T(params...);
    return obj;
}

template<typename T>
void allocator_delete(T* obj)
{
    obj->~T();
    local_allocator<T>::deallocate(obj);
}

#endif 
