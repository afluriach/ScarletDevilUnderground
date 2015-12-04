//
//  GSpace.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

const set<GType> GSpace::selfCollideTypes = list_of_typed(
    (GType::environment),
    set<GType>
);

float circleMomentOfInertia(float mass, float radius)
{
    return boost::math::constants::pi<float>()/2*pow(radius,4);
}

float rectagleMomentOfInteria(float mass, const SpaceVect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

GObject* GSpace::addObject(const ValueMap& obj)
{
    string type = obj.at("type").asString();
    GObject* gobj = GObject::constructByType(type, obj);
    
    return addObject(gobj);
}

GObject* GSpace::addObject(GObject* obj)
{
    toAdd.push_back(obj);
    
    return obj;
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
    
    //physics step
    space.step(App::secondsPerFrame);
    
    foreach(GObject* obj, objByUUID | boost::adaptors::map_values){
        obj->update();
    }
    
    processRemovals();
    
    //process additions
    processAdditions();
}


bool isSelfCollideType(GType t)
{
    return GSpace::selfCollideTypes.find(t) != GSpace::selfCollideTypes.end();
}

void setShapeProperties(shared_ptr<Shape> shape, int layers, GType type, bool sensor)
{
    shape->setLayers(layers);
    shape->setGroup(isSelfCollideType(type) ? 0 : type);
    shape->setCollisionType(type);
    shape->setSensor(sensor);
}

shared_ptr<Body> GSpace::createCircleBody(
    Space& space,
    const SpaceVect& center,
    float radius,
    float mass,
    GType type,
    int layers,
    bool sensor,
    GObject* obj)
{
    if(logPhysics) log(
        "createCircleBody for %s at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(center),
        mass
    );
    
    if(radius == 0)
        log("createCircleBody: zero radius for %s.", obj->name.c_str());
    
    shared_ptr<Body> body;
    if(mass < 0){
        body = space.makeStaticBody();
    }
    else{
        body = make_shared<Body>(mass, circleMomentOfInertia(mass, radius));
        space.add(body);
    }
    body->setPos(center);
    
    shared_ptr<CircleShape> shape = make_shared<CircleShape>(body, radius);
    shape->setBody(body);
    space.add(shape);
    
    setShapeProperties(shape, layers, type, sensor);
    
    shape->setUserData(obj);
    body->setUserData(obj);
    body->addShape(shape);
    
    return body;
}

shared_ptr<Body> GSpace::createRectangleBody(
    Space& space,
    const SpaceVect& center,
    const SpaceVect& dim,
    float mass,
    GType type,
    int layers,
    bool sensor,
    GObject* obj)
{
    if(logPhysics) log(
        "Creating rectangle body for %s. %f x %f at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(dim),
        expand_vector2(center),
        mass
    );
    
    if(dim.x == 0)
        log("createRectangleBody: zero width for %s.", obj->name.c_str());
    if(dim.y == 0)
        log("createRectangleBody: zero height for %s.", obj->name.c_str());
    
    shared_ptr<Body> body;
    if(mass < 0){
        body = space.makeStaticBody();
    }
    else{
        body = make_shared<Body>(mass, rectagleMomentOfInteria(mass, dim));
        space.add(body);
    }
    body->setPos(center);
    
    shared_ptr<PolyShape> shape = PolyShape::rectangle(body, dim);
    shape->setBody(body);
    space.add(shape);
    
    setShapeProperties(shape, layers, type, sensor);
    
    shape->setUserData(obj);
    body->setUserData(obj);
    body->addShape(shape);
    
    return body;
}

void GSpace::processAdditions()
{
    foreach(GObject* obj, toAdd)
    {
        obj->initializeBody(space);
        obj->initializeGraphics(graphicsLayer);
        
//        auto name_it = objByName.find(obj->name);
        
//        if(!obj->name.empty() && name_it != objByName.end()){
//            log("processAdditions: duplicate object with name %s", obj->name.c_str());
//        }
        objByName[obj->name] = obj;
        objByUUID[obj->uuid] = obj;
    }
    //move(toAdd.begin(), toAdd.end(), addedLastFrame.end());
    //For some strange reason move fails with a memory error here.
    foreach(GObject* obj, toAdd)
    {
        addedLastFrame.push_back(obj);
    }
    
    toAdd.clear();
}

void GSpace::removeObject(const string& name)
{
    auto it = objByName.find(name);
    if(it == objByName.end()){
        log("removeObject: %s not found", name.c_str());
        return;
    }
    
    toRemove.push_back(it->second);
}

void GSpace::removeObject(GObject* obj)
{
    //Check for object being scheduled for removal twice.
    if(find(toRemove.begin(), toRemove.end(), obj) == toRemove.end())
        toRemove.push_back(obj);
}

void GSpace::processRemovals()
{
    BOOST_FOREACH(GObject* obj, toRemove){
        objByName.erase(obj->name);
        objByUUID.erase(obj->uuid);
        obj->body->removeShapes(space);
        space.remove(obj->body);

        delete obj;
    }
    toRemove.clear();
}

map<int,string> GSpace::getUUIDNameMap()
{
    map<int,string> result;
    
    for(auto it = objByUUID.begin(); it != objByUUID.end(); ++it)
    {
        result[it->first] = it->second->name;
    }
    return result;
}

//Collision handlers
//std::function<int(Arbiter, Space&)>

#define OBJS_FROM_ARB \
    GObject* a = static_cast<GObject*>(arb.getBodyA().getUserData()); \
    GObject* b = static_cast<GObject*>(arb.getBodyB().getUserData());

void logHandler(const string& base, Arbiter& arb)
{
    OBJS_FROM_ARB
    
    log("%s: %s, %s", base.c_str(), a->name.c_str(), b->name.c_str());
}

int playerEnemyBegin(Arbiter arb, Space& spacae)
{
    logHandler("playerEnemyBegin", arb);
    return 1;
}

int playerEnemyEnd(Arbiter arb, Space& space)
{
    logHandler("playerEnemyEnd", arb);
    return 1;
}

int playerEnemyBulletBegin(Arbiter arb, Space& spacae)
{
    OBJS_FROM_ARB
    log("%s hit by %s", a->name.c_str(), b->name.c_str());
    GScene::getSpace()->removeObject(b);
    return 1;
}

int playerBulletEnemyBegin(Arbiter arb, Space& spacae)
{
    OBJS_FROM_ARB
    log("%s hit by %s", b->name.c_str(), a->name.c_str());
    GScene::getSpace()->removeObject(a);
    return 1;
}

int playerBulletEnvironment(Arbiter arb, Space& space)
{
    OBJS_FROM_ARB
    log("%s hit object %s",  a->name.c_str(), b->name.c_str());
    GScene::getSpace()->removeObject(a);
    return 1;
}

int noCollide(Arbiter arb, Space& space)
{
    return 0;
}

#define AddHandler(a,b,begin,end) \
space.addCollisionHandler(GType::a, GType::b, begin, nullptr, nullptr, end);

void GSpace::addCollisionHandlers()
{
    AddHandler(player, enemy, playerEnemyBegin, playerEnemyEnd)
    AddHandler(player, enemyBullet, playerEnemyBulletBegin, nullptr)
    AddHandler(playerBullet, enemy, playerBulletEnemyBegin, nullptr)
    AddHandler(playerBullet, environment, playerBulletEnvironment, nullptr)
    AddHandler(playerBullet,foliage,noCollide,nullptr)
    AddHandler(enemyBullet,foliage,noCollide,nullptr)
}