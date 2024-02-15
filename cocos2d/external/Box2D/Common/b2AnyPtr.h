//
//  b2AnyPtr.h
//  cocos2d_libs
//
//  Created by Toni on 2/15/24.
//

#ifndef b2AnyPtr_h
#define b2AnyPtr_h

#include <typeindex>

struct any_ptr
{
	std::type_index type;
	void* ptr;
	
	template<typename T>
	inline static any_ptr create(T* data)
	{
		return any_ptr(typeid(T), data);
	}
	
	inline any_ptr() :
		type(typeid(void)),
		ptr(nullptr)
	{}
	
	inline any_ptr(std::type_index type, void* ptr) :
		type(type),
		ptr(ptr)
	{}
	
	template<typename T>
    void set(T* data) noexcept
    {
		type = typeid(T);
		ptr = data;
    }
    
	template<typename T>
    T* get() noexcept
    {
		if(type == typeid(T)){
			return static_cast<T*>(ptr);
		}
		else{
			return nullptr;
		}
    }
};

#endif /* b2AnyPtr_h */
