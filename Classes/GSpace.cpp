//
//  GSpace.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include <cmath>
#include <memory>

#include <boost/math/constants/constants.hpp>

#include "cocos2d.h"

#include "App.h"
#include "util.h"

#include "GObject.hpp"
#include "GSpace.hpp"

using namespace std;
USING_NS_CC;

float circleMomentOfInertia(float mass, float radius)
{
    return boost::math::constants::pi<float>()/2*pow(radius,4);
}

float rectagleMomentOfInteria(float mass, const cp::Vect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
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

void GSpace::initObjects()
{
    foreach(GObject* obj, addedLastFrame)
    {
        obj->init();
    }
    addedLastFrame.clear();
}

void GSpace::update()
{
    //Run inits for recently added objects
    initObjects();
    
    //TODO Run updates for all objects
    foreach(GObject* obj, objects)
    {
        obj->update();
    }
    
    //physics step
    space.step(App::secondsPerFrame);
    
    //process additions
    processAdditions();
}

shared_ptr<cp::Body> GSpace::createCircleBody(
    cp::Space& space,
    const cp::Vect& center,
    float radius,
    float mass,
    GObject* obj)
{
    if(logPhysics) log(
        "createCircleBody for %s at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(center),
        mass
    );
    
    shared_ptr<cp::Body> body;
    if(mass < 0){
        body = space.staticBody;
    }
    else{
        body = make_shared<cp::Body>(mass, circleMomentOfInertia(mass, radius));
        space.add(body);
    }
    body->setPos(center);
    
    shared_ptr<cp::CircleShape> shape = make_shared<cp::CircleShape>(body, radius);
    shape->setBody(body);
    space.add(shape);
    
    //Set shape layer and group.
    //Set sensor
    //Set collision type.
    
    shape->setUserData(obj);
    body->setUserData(obj);
    
    return body;
}

shared_ptr<cp::Body> GSpace::createRectangleBody(
    cp::Space& space,
    const cp::Vect& center,
    const cp::Vect& dim,
    float mass,
    GObject* obj)
{
    if(logPhysics) log(
        "Creating rectangle body for %s. %f x %f at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(dim),
        expand_vector2(center),
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
        obj->initializeGraphics(graphicsLayer);
        objects.push_back(obj);
    }
    //move(toAdd.begin(), toAdd.end(), addedLastFrame.end());
    //For some strange reason std::move fails with a memory error here.
    foreach(GObject* obj, toAdd)
    {
        addedLastFrame.push_back(obj);
    }
    
    toAdd.clear();
}