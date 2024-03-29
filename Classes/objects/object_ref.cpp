//
//  object_ref.cpp
//  Koumachika
//
//  Created by Toni on 4/18/18.
//
//

#include "Prefix.h"

GObject* _object_ref_get_gobject(const GSpace* space, unsigned int uuid)
{    
    if(!space)
        return nullptr;
    
    return space->getObject(uuid);
}

bool _object_ref_is_valid(const GSpace* space, unsigned int uuid)
{   
    if(!space)
        return false;
    
    return space->isValid(uuid);
}

bool _object_ref_is_future(const GSpace* space, unsigned int uuid)
{
	if (!space)
		return false;

	return space->isFutureObject(uuid);
}

ObjectIDType _object_ref_get_uuid(const GObject* obj)
{
    if(obj == nullptr)
        return 0;
    else
        return obj->getUUID();
}

GSpace* _object_ref_get_space(const GObject* obj)
{
	if (obj == nullptr)
		return nullptr;
	else
		return obj->space;
}
