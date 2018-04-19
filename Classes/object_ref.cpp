//
//  object_ref.cpp
//  Koumachika
//
//  Created by Toni on 4/18/18.
//
//

#include "GObject.hpp"
#include "GSpace.hpp"
#include "object_ref.hpp"
#include "scenes.h"

object_ref::object_ref():
uuid(0)
{}

object_ref::object_ref(unsigned int uuid):
uuid(uuid)
{}

object_ref::object_ref(const GObject& obj):
uuid(obj.getUUID())
{}

object_ref::object_ref(GObject* obj)
{
    uuid = obj ? obj->getUUID() : 0;
}

GObject* object_ref::get()
{
    GSpace * space = GScene::getSpace();
    
    if(!space)
        return nullptr;
    
    return space->getObject(uuid);
    
}
