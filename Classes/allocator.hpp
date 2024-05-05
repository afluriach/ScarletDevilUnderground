//
//  allocator.hpp
//  Koumachika
//
//  Created by Toni on 8/14/20.
//
//

#ifndef allocator_hpp
#define allocator_hpp

#if USE_ALLOCATOR
template<typename T>
using local_allocator = boost::fast_pool_allocator<
	T,
	boost::default_user_allocator_new_delete,
	boost::details::pool::null_mutex
>;
#else
template<typename T>
using local_allocator = std::allocator<T>;
#endif

template<typename T, typename... Params>
T* allocator_new(Params... params)
{
#if USE_ALLOCATOR
	T* obj = local_allocator<T>::allocate();
	new (obj) T(params...);
	return obj;
#else
	return new T(params...);
#endif
}

template<typename T>
void allocator_delete(T* obj)
{
#if USE_ALLOCATOR
	obj->~T();
	local_allocator<T>::deallocate(obj);
#else
	delete obj;
#endif
}

#endif 
