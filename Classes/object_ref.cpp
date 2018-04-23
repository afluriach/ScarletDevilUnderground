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

GObject* _object_ref_get_gobject(unsigned int uuid)
{
    GSpace * space = GScene::getSpace();
    
    if(!space)
        return nullptr;
    
    return space->getObject(uuid);
}

bool _object_ref_is_valid(unsigned int uuid)
{
    GSpace * space = GScene::getSpace();
    
    if(!space)
        return false;
    
    return space->isValid(uuid);
}


unsigned int _object_ref_get_uuid(GObject* obj)
{
    if(obj == nullptr)
        return 0;
    else
        return obj->getUUID();
}
