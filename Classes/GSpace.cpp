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

float rectagleMomentOfInteria(float mass, const cp::Vect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

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

shared_ptr<cp::Body> GSpace::createRectangleBody(
    cp::Space& space,
    const cp::Vect& center,
    const cp::Vect& dim,
    float mass,
    GObject* obj)
{
    log(
        "Creating rectangle body for %s. %f x %f at %f,%f, mass: %f",
        obj->name.c_str(),
        dim.x, dim.y,
        center.x, center.y,
        mass
    );
    
    shared_ptr<cp::Body> body;
    if(mass < 0){
        body = space.staticBody;
    }
    else{
        body = make_shared<cp::Body>(mass, rectagleMomentOfInteria(mass, dim));
        space.add(body);
    }
    body->setPos(center);
    
    shared_ptr<cp::PolyShape> shape = cp::PolyShape::rectangle(body, dim);
    shape->setBody(body);
    space.add(shape);
    
    //Set shape layer and group.
    //Set sensor
    //Set collision type.
    
    shape->setUserData(obj);
    body->setUserData(obj);
    
    return body;
}

void GSpace::processAdditions()
{
    foreach(GObject* obj, toAdd)
    {
        obj->initializeBody(space);
    }
}