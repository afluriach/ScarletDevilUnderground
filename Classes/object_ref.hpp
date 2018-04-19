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

class object_ref
{
public:
    object_ref();
    object_ref(unsigned int uuid);
    object_ref(const GObject& obj);
    object_ref(GObject* obj);
    GObject* get();
protected:
    unsigned int uuid;
};

#endif /* object_ref_hpp */
