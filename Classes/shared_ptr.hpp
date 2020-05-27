//
//  shared_ptr.hpp
//  Koumachika
//
//  Created by Toni on 2/14/20.
//
//

#ifndef shared_ptr_hpp
#define shared_ptr_hpp

//Intrusive refcount. There is no additional allocation or extra memory overhead,
//meaning an extra field in the smart pointer to point to a control structure.
//Instead, the overhead is the refcount field on every allocated object, and the
//construction/destruction/copy semantics that apply increment/decrement the refcount.
template<typename T>
class local_shared_ptr
{
public:
	template <typename U>
	friend class local_shared_ptr;

	typedef int count_type;

	inline local_shared_ptr() {}

	inline local_shared_ptr(nullptr_t){
	}

	//Normally, create would be called here so that a shared object entry or counter of
	//some type can be created. In this case, the use of an intrusive refcount means there is no need.
	//This works for creating a new object, but also shared from this, or otherwise converting 
	//raw ptr back to local_shared_ptr
	inline local_shared_ptr(T* t)
	{
		obj = t;
		acquire();
		//create();
	}

	inline local_shared_ptr(const local_shared_ptr<T>& rhs)
	{
		copy(rhs);
	}

	//"Copy" constructor, but actually static upcast
	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr(const local_shared_ptr<U>& rhs)
	{
		copy(rhs);
	}

	inline local_shared_ptr(local_shared_ptr<T>&& rhs)
	{
		obj = rhs.obj;
		rhs.obj = nullptr;
	}

	//Move existing
	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr(local_shared_ptr<U>&& rhs)
	{
		obj = rhs.obj;
		rhs.obj = nullptr;
	}

	inline ~local_shared_ptr()
	{
		reset();
	}

	inline local_shared_ptr<T>& operator=(const local_shared_ptr<T>& rhs) {
		copy(rhs);
		return *this;
	}

	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr<T>& operator=(const local_shared_ptr<U>& rhs) {
		copy(rhs);
		return *this;
	}

	inline void reset() {
		if (obj) {
			release();
		}
	}

	template<typename U>
	inline local_shared_ptr<U> downcast() {
		local_shared_ptr<U> result;

		result.obj = dynamic_cast<U*>(obj);

		if (result.obj)
			acquire();

		return result;
	}

	inline bool operator==(const local_shared_ptr<T>& rhs) const {
		return obj == rhs.obj;
	}

	inline T* get() const {
		return obj;
	}

	inline T& operator*() const {
		return *obj;
	}

	inline T* operator->() const {
		return obj;
	}

	inline operator bool() const {
		return obj != nullptr;
	}

protected:
	template<typename U>
	inline void copy(const local_shared_ptr<U>& rhs) {
		reset();

		obj = rhs.obj;

		if (obj)
			acquire();
	}

	//inline void create()
	//{
	//	obj->_refcount = 1;
	//}

	inline void acquire()
	{
		++obj->_refcount;
	}

	inline void release()
	{
		_release();

		bool destruct = obj->_refcount <= 0;

		if (destruct) {
			allocator_delete<T>(obj);
		}

		obj = nullptr;
	}

	inline void _release()
	{
		--obj->_refcount;
	}

	T* obj = nullptr;
};

template<typename T, typename... Params>
inline local_shared_ptr<T> make_local_shared(Params... params)
{
	T* obj = allocator_new<T>(params...);
	return local_shared_ptr<T>(obj);
}

#endif 
