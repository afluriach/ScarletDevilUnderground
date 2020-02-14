//
//  object_ref.hpp
//  Koumachika
//
//  Created by Toni on 4/18/18.
//
//

#ifndef object_ref_hpp
#define object_ref_hpp

class GObject;
class GSpace;

GObject* _object_ref_get_gobject(const GSpace* space, unsigned int uuid);
bool _object_ref_is_valid(const GSpace* space, unsigned int uuid);
bool _object_ref_is_future(const GSpace* space, unsigned int uuid);
ObjectIDType _object_ref_get_uuid(const GObject* obj);
GSpace* _object_ref_get_space(const GObject* obj);

class gobject_ref
{
public:
    inline gobject_ref():
    uuid(0),
	space(nullptr)
    {}

	inline gobject_ref(const gobject_ref& rhs) :
		space(rhs.getSpace()),
		uuid(rhs.getID())
	{}

    inline gobject_ref(const GSpace* space, unsigned int uuid):
    uuid(uuid),
	space(space)
    {}

	gobject_ref(const GObject* obj) :
		uuid(_object_ref_get_uuid(obj)),
		space(_object_ref_get_space(obj))
	{}

	template<class T>
    inline T* getAs() const{
        return dynamic_cast<T*>(_object_ref_get_gobject(space,uuid));
    }

    inline GObject* get() const{
		return _object_ref_get_gobject(space, uuid);
	}

    inline bool isValid()const{
        return _object_ref_is_valid(space,uuid);
    }

	inline bool isFuture()const {
		return _object_ref_is_future(space, uuid);
	}
    
    inline bool operator==(const GObject* rhs)const{
        if(!rhs)
            return uuid == 0;
        else
            return space == _object_ref_get_space(rhs) && uuid == _object_ref_get_uuid(rhs);
    }

	inline bool operator==(const gobject_ref& rhs)const {
		return this->space == rhs.space && this->uuid == rhs.uuid;
	}

	inline bool operator!=(const gobject_ref& rhs)const {
		return this->space != rhs.space || this->uuid != rhs.uuid;
	}

	inline bool operator<(const gobject_ref& rhs) const {
		return this->uuid < rhs.uuid;
	}

	inline ObjectIDType getID() const {
		return uuid;
	}

	inline const GSpace* getSpace() const {
		return space;
	}

protected:
	const GSpace * space;
    ObjectIDType uuid;
};

namespace std {
	template<>
	struct hash<gobject_ref>
	{
		inline size_t operator()(const gobject_ref& k) const {
			return std::hash<ObjectIDType>{}(k.getID());
		}
	};
}

#endif /* object_ref_hpp */
