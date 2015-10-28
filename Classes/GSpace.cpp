//
//  GSpace.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "cocos2d.h"

#include "util.h"

#include "GObject.hpp"
#include "GSpace.hpp"

using namespace std;
USING_NS_CC;

GSpace::GSpace()
{
    space.setGravity(cp::Vect(0,0));
}

void GSpace::addObject(const ValueMap& obj)
{
    //    GObject gobj(obj);
    string type = obj.at("type").asString();
    GObject* gobj = GObject::constructByType(type, obj);
    
    toAdd.push_back(gobj);
}

void GSpace::addObjects(const ValueVector& objs)
{
    foreach(Value obj, objs)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        addObject(objAsMap);
    }
}
