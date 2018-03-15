//
//  GSpace.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"
#include "Bullet.hpp"

class RadarObject;

const set<GType> GSpace::selfCollideTypes = list_of_typed(
    (GType::environment)
    (GType::enemy),
    set<GType>
);

float circleMomentOfInertia(float mass, float radius)
{
    return float_pi/2*pow(radius,4);
}

float rectagleMomentOfInteria(float mass, const SpaceVect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

GSpace::GSpace(Layer* graphicsLayer) : graphicsLayer(graphicsLayer)
{
    GObject::resetObjectUUIDs();

    space.setGravity(SpaceVect(0,0));
    addCollisionHandlers();
}

GSpace::~GSpace()
{
    //Avoid calling seperation (or "end contact") handlers.
    cp::Space::maskSeperateHandler = true;

    //Process removal modified objByUUID.
    vector<GObject*> objs;
    
    for(auto it = objByUUID.begin(); it != objByUUID.end(); ++it){
        objs.push_back(it->second);
    }
    
    foreach(GObject* obj, objs){
        processRemoval(obj);
    }
    
    if(navMask)
        delete navMask;

    cp::Space::maskSeperateHandler = false;
}

GObject* GSpace::addObject(const ValueMap& obj)
{
    string type = obj.at("type").asString();
    GObject* gobj = GObject::constructByType(type, obj);
    
    return addObject(gobj);
}

GObject* GSpace::addObject(GObject* obj)
{
    if(obj)
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
    
    ++frame;
}


bool isSelfCollideType(GType t)
{
    return GSpace::selfCollideTypes.find(t) != GSpace::selfCollideTypes.end();
}

void setShapeProperties(shared_ptr<Shape> shape, PhysicsLayers layers, GType type, bool sensor)
{
    shape->setLayers(static_cast<unsigned int>(layers));
    shape->setGroup(static_cast<unsigned int>(type));
    shape->setCollisionType(static_cast<unsigned int>(type));
    shape->setSensor(sensor);
    shape->setSelfCollide(isSelfCollideType(type));
}

shared_ptr<Body> GSpace::createCircleBody(
    const SpaceVect& center,
    float radius,
    float mass,
    GType type,
    PhysicsLayers layers,
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
        if(type == GType::environment)
            addNavObstacle(center, SpaceVect(radius*2, radius*2));
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
    const SpaceVect& center,
    const SpaceVect& dim,
    float mass,
    GType type,
    PhysicsLayers layers,
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
        if(type == GType::environment)
            addNavObstacle(center,dim);
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

void GSpace::addWallBlock(SpaceVect ll,SpaceVect ur)
{
    SpaceVect center = (ll + ur) / 2;
    SpaceVect dim = ur - ll;
    
    shared_ptr<Body> wallBlock = createRectangleBody(
        center,
        dim,
        -1,
        GType::wall,
        PhysicsLayers::all,
        false,
        nullptr
    );
    
    space.add(wallBlock);
}

void GSpace::addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions)
{
    for(float x = center.x - boundingDimensions.x/2; x < center.x + boundingDimensions.x/2; ++x)
    {
        for(float y = center.y - boundingDimensions.y/2; y < center.y + boundingDimensions.y/2; ++y)
        {
            markObstacleTile(x,y);
        }
    }
}

bool GSpace::isObstacle(IntVec2 v)
{
    return isObstacleTile(v.first, v.second);
}

void GSpace::processAdditions()
{
    foreach(GObject* obj, toAdd)
    {
        obj->initializeBody(*this);
        obj->initializeRadar(*this);
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

void GSpace::processRemoval(GObject* obj)
{
    objByName.erase(obj->name);
    objByUUID.erase(obj->uuid);
    
    obj->body->removeShapes(space);
    space.remove(obj->body);
    
    if(obj->radar){
        obj->radar->removeShapes(space);
        space.remove(obj->radar);
    }

    delete obj;
}

void GSpace::processRemovals()
{
    BOOST_FOREACH(GObject* obj, toRemove){
        processRemoval(obj);
    }
    toRemove.clear();
}

unordered_map<int,string> GSpace::getUUIDNameMap()
{
    unordered_map<int,string> result;
    
    for(auto it = objByUUID.begin(); it != objByUUID.end(); ++it)
    {
        result[it->first] = it->second->name;
    }
    return result;
}

float GSpace::distanceFeeler(GObject* agent, SpaceVect _feeler, GType gtype)
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
    
    //Distance along the segment is scaled [0,1].
    float closest = 1.0f;
    
    auto queryCallback = [&closest,agent] (std::shared_ptr<Shape> shape, cp::Float distance, cp::Vect vect) -> void {
        
        if(shape->getUserData() != agent){
            closest = min<float>(closest, distance);
        }
    };
    
    space.segmentQuery(
        start,
        end,
        static_cast<unsigned int>(PhysicsLayers::all),
        static_cast<unsigned int>(gtype),
        queryCallback);
    
    return closest*_feeler.length();
}

float GSpace::wallDistanceFeeler(GObject* agent, SpaceVect feeler)
{
    return distanceFeeler(agent, feeler, GType::wall);
}

float GSpace::obstacleDistanceFeeler(GObject* agent, SpaceVect _feeler)
{
    return vmin(
        wallDistanceFeeler(agent, _feeler),
        distanceFeeler(agent, _feeler, GType::environment),
        distanceFeeler(agent, _feeler, GType::enemy)
    );
}

bool GSpace::feeler(GObject* agent, SpaceVect _feeler, GType gtype)
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
    
    bool collision = false;
    
    auto queryCallback = [agent, &collision] (std::shared_ptr<Shape> shape, cp::Float distance, cp::Vect vect) -> void {
        
        if(shape->getUserData() != agent){
            collision = true;
        }
    };
    
    space.segmentQuery(
        start,
        end,
        static_cast<unsigned int>(PhysicsLayers::all),
        static_cast<unsigned int>(gtype),
        queryCallback);
    
    return collision;
}

bool GSpace::wallFeeler(GObject* agent, SpaceVect _feeler)
{
    return feeler(agent, _feeler, GType::wall);
}

bool GSpace::obstacleFeeler(GObject* agent, SpaceVect _feeler)
{
    return
        wallFeeler(agent, _feeler) ||
        feeler(agent, _feeler, GType::environment) ||
        feeler(agent, _feeler, GType::enemy)
    ;
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
    OBJS_FROM_ARB
    
    Player* p = dynamic_cast<Player*>(a);
    Enemy* e = dynamic_cast<Enemy*>(b);
    
    if(!p)
        log("%s is not a Player", a->getName().c_str());
    if(!e)
        log("%s is not an Enemy", b->getName().c_str());
    
    if(p && e)
        e->onTouchPlayer(p);

    return 1;
}

int playerEnemyEnd(Arbiter arb, Space& space)
{
	OBJS_FROM_ARB

	Enemy* e = dynamic_cast<Enemy*>(b);

	if(e)
		e->endTouchPlayer();
    
	logHandler("playerEnemyEnd", arb);
    return 1;
}

int playerEnemyBulletBegin(Arbiter arb, Space& spacae)
{
    GObject* playerObj = static_cast<GObject*>(arb.getBodyA().getUserData());
    GObject* bullet = static_cast<GObject*>(arb.getBodyB().getUserData());
    Player* player = dynamic_cast<Player*>(playerObj);
    
    log("%s hit by %s", player->name.c_str(), bullet->name.c_str());
    player->hit();
    GScene::getSpace()->removeObject(bullet);
    return 1;
}

int playerBulletEnemyBegin(Arbiter arb, Space& spacae)
{
    OBJS_FROM_ARB
    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Enemy* enemy = dynamic_cast<Enemy*>(b);
    
    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!enemy)
        log("%s is not an Enemy", b->getName().c_str());
    
    if(bullet && enemy)
        enemy->onPlayerBulletHit(bullet);
    
    log("%s hit by %s", b->name.c_str(), a->name.c_str());
    GScene::getSpace()->removeObject(a);
    return 1;
}

int playerFlowerBegin(Arbiter arb, Space& spacae)
{
    OBJS_FROM_ARB
    log("%s stepped on", b->name.c_str());
    return 1;
}

int bulletEnvironment(Arbiter arb, Space& space)
{
    OBJS_FROM_ARB
//    log("%s hit object %s",  a->name.c_str(), b->name.c_str());
    GScene::getSpace()->removeObject(a);
    return 1;
}

int noCollide(Arbiter arb, Space& space)
{
    return 0;
}

int bulletWall(Arbiter arb, Space& space)
{
    GObject* bullet = static_cast<GObject*>(arb.getBodyA().getUserData());
    GScene::getSpace()->removeObject(bullet);
    return 1;
}

int sensorStart(Arbiter arb, Space& space)
{
    GObject* radarAgent = static_cast<GObject*>(arb.getBodyA().getUserData());
    GObject* target = static_cast<GObject*>(arb.getBodyB().getUserData());
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);

	if (radarObject) {
		log("%s sensed %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->onDetect(target);
	}
	else {
		log("sensorStart: %s is not a radar object", radarAgent->name.c_str());
	}

	return 1;
}

int sensorEnd(Arbiter arb, Space& space)
{
    GObject* radarAgent = static_cast<GObject*>(arb.getBodyA().getUserData());
    GObject* target = static_cast<GObject*>(arb.getBodyB().getUserData());
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);
    
	if (radarObject) {
		log("%s lost %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->onEndDetect(target);
	}
	else {
		log("sensorEnd: %s is not a radar object", radarAgent->name.c_str());
	}

    return 1;
}


#define AddHandler(a,b,begin,end) \
space.addCollisionHandler(static_cast<CollisionType>(GType::a), static_cast<CollisionType>(GType::b), begin, nullptr, nullptr, end);

void GSpace::addCollisionHandlers()
{
    AddHandler(player, enemy, playerEnemyBegin, playerEnemyEnd)
    AddHandler(player, enemyBullet, playerEnemyBulletBegin, nullptr)
    AddHandler(playerBullet, enemy, playerBulletEnemyBegin, nullptr)
    AddHandler(playerBullet, environment, bulletEnvironment, nullptr)
    AddHandler(enemyBullet, environment, bulletEnvironment, nullptr)
    AddHandler(playerBullet,foliage,noCollide,nullptr)
    AddHandler(enemyBullet,foliage,noCollide,nullptr)
    AddHandler(playerBullet,enemyBullet, noCollide,nullptr)
    AddHandler(player, foliage, playerFlowerBegin,nullptr)
    
    AddHandler(playerBullet, wall, bulletWall, nullptr);
    AddHandler(enemyBullet, wall, bulletWall, nullptr);
    
    AddHandler(playerSensor, player, sensorStart, sensorEnd);

    AddHandler(objectSensor, enemy, sensorStart, sensorEnd)
    AddHandler(objectSensor, environment, sensorStart, sensorEnd);
}
