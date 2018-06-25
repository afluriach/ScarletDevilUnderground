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

GObject* _object_ref_get_gobject(unsigned int uuid);
bool _object_ref_is_valid(unsigned int uuid);
unsigned int _object_ref_get_uuid(const GObject* obj);

template<class T>
class object_ref
{
public:
    inline object_ref():
    uuid(0)
    {}

    inline object_ref(unsigned int uuid):
    uuid(uuid)
    {}

    inline object_ref(const T& obj):
    uuid(obj.getUUID())
    {}

    inline object_ref(T* obj){
        uuid = _object_ref_get_uuid(obj);
    }

    inline T* get() const{
        return dynamic_cast<T*>(_object_ref_get_gobject(uuid));
    }

    inline GObject* getBase() const{
        return get();
    }

    inline bool isValid()const{
        return _object_ref_is_valid(uuid);
    }
    
    inline bool operator==(const GObject* rhs)const{
        if(!rhs)
            return uuid == 0;
        else
            return uuid == _object_ref_get_uuid(rhs);
    }

    inline bool operator==(const GObject& rhs)const{
        return this == &rhs;
    }

protected:
    unsigned int uuid;
};

typedef object_ref<GObject> gobject_ref;

#endif /* object_ref_hpp */
