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

	static unique_ptr<shared_ptr_system> inst;
	static shared_ptr_system* get();

	shared_ptr_system();

	void acquire(id_type id);
	id_type create();
	bool release(id_type id);

protected:

	unordered_map<id_type, count_type> refs;
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
		id = shared_ptr_system::get()->create();
	}

	inline local_shared_ptr(const local_shared_ptr<T>& rhs)
	{
		obj = rhs.obj;
		id = rhs.id;

		if (id != 0)
			shared_ptr_system::get()->acquire(id);
	}

	//"Copy" constructor, but actually static upcast
	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr(const local_shared_ptr<U>& rhs)
	{
		obj = rhs.obj;
		id = rhs.id;

		if(id != 0)
			shared_ptr_system::get()->acquire(id);
	}

	inline local_shared_ptr(local_shared_ptr<T>&& rhs)
	{
		obj = rhs.obj;
		id = rhs.id;

		rhs.obj = nullptr;
		rhs.id = 0;
	}

	//Move existing
	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr(local_shared_ptr<U>&& rhs)
	{
		obj = rhs.obj;
		id = rhs.id;

		rhs.obj = nullptr;
		rhs.id = 0;
	}

	inline ~local_shared_ptr()
	{
		if(id != 0)
			release();
	}

	inline local_shared_ptr<T>& operator=(const local_shared_ptr<T>& rhs) {
		if (id != 0) {
			release();
		}

		obj = rhs.obj;
		id = rhs.id;

		if (id != 0)
			shared_ptr_system::get()->acquire(id);

		return *this;
	}

	template<typename U, enable_if_t<is_base_of<T, U>::value, int> = 0>
	inline local_shared_ptr<T>& operator=(const local_shared_ptr<U>& rhs) {
		if (id != 0) {
			release();
		}

		obj = rhs.obj;
		id = rhs.id;

		if(id != 0)
			shared_ptr_system::get()->acquire(id);

		return *this;
	}

	inline void reset() {
		if (id != 0) {
			release();
		}
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

	inline void release()
	{
		bool destruct = shared_ptr_system::inst && shared_ptr_system::get()->release(id);

		if (destruct) {
			delete obj;
		}

		obj = nullptr;
		id = 0;
	}

	T* obj = nullptr;
	shared_ptr_system::id_type id = 0;
};

template<typename T, typename... Params>
inline local_shared_ptr<T> make_local_shared(Params... params)
{
	return local_shared_ptr<T>(new T(params...));
}

#endif 
