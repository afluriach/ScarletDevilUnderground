//
//  shared_ptr.hpp
//  Koumachika
//
//  Created by Toni on 2/14/20.
//
//

#ifndef shared_ptr_hpp
#define shared_ptr_hpp

class shared_ptr_system
{
public:
	typedef unsigned int id_type;
	typedef unsigned int count_type;

	struct shared_object_entry
	{
		inline shared_object_entry(id_type id) :
			id(id),
			count(1)
		{}

		id_type id;
		count_type count;
	};

	static unique_ptr<shared_ptr_system> inst;
	static shared_ptr_system* get();

	shared_ptr_system();

	void acquire(shared_object_entry* shared);
	shared_object_entry* create();
	bool release(shared_object_entry* entry);

protected:

	unordered_map<id_type, shared_object_entry*> refs;
	id_type nextID = 1;
};

template<typename T>
class local_shared_ptr
{
public:
	template <typename U>
	friend class local_shared_ptr;

	inline local_shared_ptr() {}

	inline local_shared_ptr(nullptr_t){
	}

	inline local_shared_ptr(T* t)
	{
		obj = t;
		shared = shared_ptr_system::get()->create();
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
		shared = rhs.shared;

		rhs.obj = nullptr;
		rhs.shared = nullptr;
	}

	//Move existing
	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr(local_shared_ptr<U>&& rhs)
	{
		obj = rhs.obj;
		shared = rhs.shared;

		rhs.obj = nullptr;
		rhs.shared = nullptr;
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
		if (shared) {
			release();
		}
	}

	template<typename U>
	inline local_shared_ptr<U> downcast() {
		local_shared_ptr<U> result;

		result.obj = dynamic_cast<U*>(obj);
		result.shared = shared;

		if (!result.obj)
			result.shared = nullptr;

		if (result.obj && shared)
			shared_ptr_system::get()->acquire(shared);

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
		shared = rhs.shared;

		if (shared)
			shared_ptr_system::get()->acquire(shared);
	}

	inline void release()
	{
		bool destruct = shared_ptr_system::inst && shared && shared_ptr_system::get()->release(shared);

		if (destruct) {
			allocator_delete<T>(obj);
		}

		obj = nullptr;
		shared = nullptr;
	}

	T* obj = nullptr;
	shared_ptr_system::shared_object_entry* shared = nullptr;
};

template<typename T, typename... Params>
inline local_shared_ptr<T> make_local_shared(Params... params)
{
	T* obj = allocator_new<T>(params...);
	return local_shared_ptr<T>(obj);
}

#endif 
