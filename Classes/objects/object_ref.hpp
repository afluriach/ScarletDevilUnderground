//
//  object_ref.hpp
//  Koumachika
//
//  Created by Toni on 4/18/18.
//
//

#ifndef object_ref_hpp
#define object_ref_hpp

#include "types.h"

class GObject;
class GSpace;

GObject* _object_ref_get_gobject(GSpace* space, unsigned int uuid);
bool _object_ref_is_valid(GSpace* space, unsigned int uuid);
ObjectIDType _object_ref_get_uuid(const GObject* obj);
GSpace* _object_ref_get_space(const GObject* obj);

template<class T>
class object_ref
{
public:
    inline object_ref():
    uuid(0),
	space(nullptr)
    {}

    inline object_ref(GSpace* space, unsigned int uuid):
    uuid(uuid),
	space(space)
    {}

	object_ref(GObject* obj) :
		uuid(_object_ref_get_uuid(obj)),
		space(_object_ref_get_space(obj))
	{}

	inline object_ref(const object_ref<GObject>& ref) : 
		uuid(ref.getID()),
		space(ref.getSpace())
	{}

    inline T* get() const{
        return dynamic_cast<T*>(_object_ref_get_gobject(space,uuid));
    }

    inline GObject* getBase() const{
        return get();
    }

    inline bool isValid()const{
        return _object_ref_is_valid(space,uuid);
    }
    
    inline bool operator==(const GObject* rhs)const{
        if(!rhs)
            return uuid == 0;
        else
            return this->space && rhs->space && uuid == _object_ref_get_uuid(rhs);
    }

    inline bool operator==(const GObject& rhs)const{
        return this == &rhs;
    }

	inline bool operator==(const object_ref<T>& rhs)const {
		return this->space == rhs.space && this->uuid == rhs.uuid;
	}

	inline bool operator!=(const object_ref<T>& rhs)const {
		return this->space != rhs.space || this->uuid != rhs.uuid;
	}

	inline ObjectIDType getID() const {
		return uuid;
	}

	inline GSpace* getSpace() const {
		return space;
	}

protected:
	GSpace * space;
    ObjectIDType uuid;
};

typedef object_ref<GObject> gobject_ref;

#endif /* object_ref_hpp */
