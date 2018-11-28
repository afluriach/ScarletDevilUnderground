//
//  GSpace.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "AIMixins.hpp"
#include "App.h"
#include "Bullet.hpp"
#include "Collectibles.hpp"
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Player.hpp"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"
#include "Wall.hpp"

class RadarObject;

GSpace::GSpace(Layer* graphicsLayer) : graphicsLayer(graphicsLayer)
{
    GObject::resetObjectUUIDs();

    space.setGravity(SpaceVect(0,0));
    addCollisionHandlers();
    app->space = this;
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
        processRemoval(obj, true);
    }
    
    if(navMask)
        delete navMask;

    cp::Space::maskSeperateHandler = false;
}

void GSpace::setSize(int x, int y)
{
    spaceSize = IntVec2(x,y);
    if(navMask)
        delete navMask;
    navMask = new boost::dynamic_bitset<>(x*y);
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

	foreach(GObject* obj, objByUUID | boost::adaptors::map_values) {
		obj->applyPhysicsProperties();
	}
    
    processRemovals();
    
    //process additions
    processAdditions();
    
    ++frame;
}

//BEGIN OBJECT MANIPULATION

const bool GSpace::logObjectArgs = false;

GObject* GSpace::addObject(const ValueMap& obj)
{
    if(logObjectArgs)
        printValueMap(obj);

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

void GSpace::addWallBlock(SpaceVect ll,SpaceVect ur)
{
    SpaceVect center = (ll + ur) / 2;
    SpaceVect dim = ur - ll;
    
    GObject* wall = new Wall(center,dim);
    
    shared_ptr<Body> wallBlock = createRectangleBody(
        center,
        dim,
        -1,
        GType::wall,
        PhysicsLayers::all,
        false,
        wall
    );
    
    space.add(wallBlock);
    addObject(wall);
}

GObject* GSpace::getObject(const string& name) const
{
    auto it = objByName.find(name);
    return it != objByName.end() ? it->second : nullptr;
}

GObject* GSpace::getObject(unsigned int uuid) const
{
    auto it = objByUUID.find(uuid);
    return it != objByUUID.end() ? it->second : nullptr;
}

bool GSpace::isValid(unsigned int uuid) const
{
    return getObject(uuid) != nullptr;
}

vector<string> GSpace::getObjectNames() const
{
    auto key = [](pair<string,GObject*> e){return e.first;};
    vector<string> names(objByName.size());
    transform(objByName.begin(), objByName.end(), names.begin(), key);
    return names;
}

void GSpace::processAdditions()
{
    foreach(GObject* obj, toAdd)
    {
        if(!obj->anonymous && objByName.find(obj->name) != objByName.end()){
            log("Object %s, %d name is not unique!", obj->name.c_str(), obj->uuid);
            delete obj;
            continue;
        }

        if(objByUUID.find(obj->uuid) != objByUUID.end()){
            log("Object %s, %d UUID is not unique!", obj->name.c_str(), obj->uuid);
            delete obj;
            continue;
        }

        obj->initializeBody(*this);
        obj->initializeRadar(*this);
        obj->initializeGraphics(graphicsLayer);
        
        if(!obj->anonymous)
            objByName[obj->name] = obj;
        objByUUID[obj->uuid] = obj;
		currentContacts[obj] = list<contact>();
        
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

void GSpace::removeObjectWithAnimation(GObject* obj, FiniteTimeAction* action)
{
	toRemoveWithAnimation.push_back(pair<GObject*, FiniteTimeAction*>(obj,action));
}

void GSpace::processRemoval(GObject* obj, bool removeSprite)
{
    objByName.erase(obj->name);
    objByUUID.erase(obj->uuid);
	currentContacts.erase(obj);
    
	if (obj->crntFloor.get())
		obj->crntFloor.get()->onEndContact(obj);

    obj->body->removeShapes(space);
    space.remove(obj->body);
    
    if(obj->radar){
        obj->radar->removeShapes(space);
        space.remove(obj->radar);
    }

	if (obj->crntSpell.get()) {
		obj->crntSpell.get()->end();
	}

	if (removeSprite && obj->sprite)
		obj->sprite->removeFromParent();

	delete obj;
}

void GSpace::initObjects()
{
    foreach(GObject* obj, addedLastFrame)
    {
        obj->init();
    }
    addedLastFrame.clear();
}

void GSpace::processRemovals()
{
	//Objects which will be removed this frame should have the end contact handlers called 
	//before they are deleted.
	foreach(GObject* obj, toRemove) {
		processRemovalEndContact(obj);
	}

	for (auto it = toRemoveWithAnimation.begin(); it != toRemoveWithAnimation.end(); ++it) {
		processRemovalEndContact(it->first);
	}

	space.maskSeperateHandler = true;

	BOOST_FOREACH(GObject* obj, toRemove){
        processRemoval(obj, true);
    }
    toRemove.clear();

	for (auto it = toRemoveWithAnimation.begin(); it != toRemoveWithAnimation.end(); ++it) {
		Node* sprite = it->first->sprite;
		processRemoval(it->first, false);
		sprite->runAction(Sequence::createWithTwoActions(it->second, RemoveSelf::create() ));
	}
	toRemoveWithAnimation.clear();

	space.maskSeperateHandler = false;
}

unordered_map<int,string> GSpace::getUUIDNameMap() const
{
    unordered_map<int,string> result;
    
    for(auto it = objByUUID.begin(); it != objByUUID.end(); ++it)
    {
        result[it->first] = it->second->name;
    }
    return result;
}
//END OBJECT MANIPULATION

//BEGIN NAVIGATION
void GSpace::addPath(string name, Path p)
{
	if (paths.find(name) != paths.end()) {
		log("Duplicate path name %s!", name.c_str());
	}
	paths[name] = p;
}

void GSpace::addRoom(cocos2d::CCRect rect)
{
	rooms.push_back(rect);
}

const Path* GSpace::getPath(string name) const
{
	auto it = paths.find(name);

	return it != paths.end() ? &(it->second) : nullptr;
}


FloorSegment* GSpace::floorSegmentPointQuery(SpaceVect pos)
{
	return dynamic_cast<FloorSegment*>(pointQuery(pos, GType::floorSegment, PhysicsLayers::floor));
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

bool GSpace::isObstacle(IntVec2 v) const
{
    return isObstacleTile(v.first, v.second);
}

void GSpace::markObstacleTile(int x, int y)
{
    if(x >= 0 && x < spaceSize.first){
        if(y >= 0 && y < spaceSize.second){
            (*navMask)[y*spaceSize.first+x] = 1;
        }
    }
}

bool GSpace::isObstacleTile(int x, int y) const
{
    if(x >= 0 && x < spaceSize.first){
        if(y >= 0 && y < spaceSize.second){
            return (*navMask)[y*spaceSize.first+x];
        }
    }
    return false;
}

//END NAVIGATION

//BEGIN PHYSICS
#define _addHandler(a,b,begin,end) AddHandler<GType::a, GType::b>(&GSpace::begin,&GSpace::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler<GType::a, GType::b>(&GSpace::begin,nullptr)

void GSpace::addCollisionHandlers()
{
	_addHandler(player, enemy, playerEnemyBegin, playerEnemyEnd);
	_addHandlerNoEnd(player, enemyBullet, playerEnemyBulletBegin);
	_addHandlerNoEnd(playerBullet, enemy, playerBulletEnemyBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, foliage, noCollide);
	_addHandlerNoEnd(enemyBullet, foliage, noCollide);
	_addHandlerNoEnd(playerBullet, enemyBullet, noCollide);
	_addHandlerNoEnd(player, foliage, playerFlowerBegin);
    _addHandlerNoEnd(player,collectible,playerCollectibleBegin);
    _addHandlerNoEnd(player,npc,collide);
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);    

	_addHandler(playerSensor, player, sensorStart, sensorEnd);
	_addHandler(playerSensor, playerBullet, sensorStart, sensorEnd);
	_addHandler(objectSensor, enemy, sensorStart, sensorEnd);
	_addHandler(objectSensor, environment, sensorStart, sensorEnd);
    _addHandler(objectSensor, npc, sensorStart, sensorEnd);
}

const set<GType> GSpace::selfCollideTypes = boost::assign::list_of
    (GType::enemy)
    (GType::environment)
    (GType::npc)
;

const bool GSpace::logBodyCreation = false;
const bool GSpace::logPhysicsHandlers = false;

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
    if(logBodyCreation) log(
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
    if(logBodyCreation && obj) log(
        "Creating rectangle body for %s. %f x %f at %f,%f, mass: %f",
        obj->name.c_str(),
        expand_vector2(dim),
        expand_vector2(center),
        mass
    );
    
    if(dim.x == 0 && obj)
        log("createRectangleBody: zero width for %s.", obj->name.c_str());
    if(dim.y == 0 && obj)
        log("createRectangleBody: zero height for %s.", obj->name.c_str());
    
    shared_ptr<Body> body;
    if(mass < 0){
        body = space.makeStaticBody();
        if(type == GType::environment)
            addNavObstacle(center,dim);
    }
    else{
        body = make_shared<Body>(mass, rectangleMomentOfInertia(mass, dim));
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

void GSpace::addContact(contact c)
{
	currentContacts[c.first.first].push_back(c);
	currentContacts[c.first.second].push_back(c);
}

void GSpace::removeContact(contact c)
{
	currentContacts[c.first.first].remove(c);
	currentContacts[c.first.second].remove(c);
}

//By making a copy of the current object's contact list, we can mutate the lists stored in the map  
//inside of the list foreach i.e. removeContact will also mutate the current object's list.
//
//After the contact is processed, remove it from both objects' contact lists.
//This will prevent a double call to the end contact handler in case both are removed in the same frame.
//
//The contact list of the object being processed should be empty when finished.
void GSpace::processRemovalEndContact(GObject* obj)
{
	list<contact> contactList = currentContacts[obj];

	foreach(contact c, contactList) {
		auto itt = endContactHandlers.find(c.second);
		if (itt != endContactHandlers.end() && itt->second) {
			int(GSpace::*end_method)(GObject*, GObject*) = itt->second;
			(this->*end_method)(c.first.first, c.first.second);
		}
		removeContact(c);
	}

	if (currentContacts[obj].size() != 0)
		log("processRemovalEndContact: object %s problem!", obj->getName().c_str());
}


void GSpace::logHandler(const string& base, Arbiter& arb)
{
    if(logPhysicsHandlers){
        OBJS_FROM_ARB
        
        log("%s: %s, %s", base.c_str(), a->name.c_str(), b->name.c_str());
    }
}

void GSpace::logHandler(const string& name, GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s: %s, %s", name.c_str(), a->name.c_str(), b->name.c_str());
}

int GSpace::playerEnemyBegin(GObject* a, GObject* b)
{    
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

int GSpace::playerEnemyEnd(GObject* a, GObject* b)
{
	Enemy* e = dynamic_cast<Enemy*>(b);

	if(e)
		e->endTouchPlayer();
    
	logHandler("playerEnemyEnd", a,b);
    return 1;
}

int GSpace::playerEnemyBulletBegin(GObject* playerObj, GObject* bullet)
{
    Player* player = dynamic_cast<Player*>(playerObj);
    
    if(logPhysicsHandlers)
        log("%s hit by %s", player->name.c_str(), bullet->name.c_str());
    player->hit();
    removeObject(bullet);
    return 1;
}

int GSpace::playerBulletEnemyBegin(GObject* a, GObject* b)
{    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Enemy* enemy = dynamic_cast<Enemy*>(b);
    
    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!enemy)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (bullet && enemy)
	{
		enemy->onPlayerBulletHit(bullet);

		shared_ptr<MagicEffect> effect = bullet->getMagicEffect(enemy);

		if (effect) {
			enemy->addMagicEffect(effect);
		}
	}

    if(logPhysicsHandlers)
        log("%s hit by %s", b->name.c_str(), a->name.c_str());
    
    removeObject(a);
    return 1;
}

int GSpace::playerFlowerBegin(GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s stepped on", b->name.c_str());
    
    return 1;
}

int GSpace::playerCollectibleBegin(GObject* a, GObject* b)
{
    Player* p = dynamic_cast<Player*>(a);
    Collectible* c = dynamic_cast<Collectible*>(b);
    
    if(p && c){
        p->onCollectible(c);
    }
    
    return 0;
}

int GSpace::bulletEnvironment(GObject* a, GObject* b)
{
    if(logPhysicsHandlers)
        log("%s hit object %s",  a->name.c_str(), b->name.c_str());
    
    removeObject(a);
    return 1;
}

int GSpace::noCollide(GObject* a, GObject* b)
{
    return 0;
}

int GSpace::collide(GObject* a, GObject* b)
{
    return 1;
}

int GSpace::bulletWall(GObject* bullet, GObject* unused)
{
    removeObject(bullet);
    return 1;
}

int GSpace::sensorStart(GObject* radarAgent, GObject* target)
{
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);

	if (radarObject) {
        if(logPhysicsHandlers)
            log("%s sensed %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->radarCollision(target);
	}
	else {
		log("sensorStart: %s is not a radar object", radarAgent->name.c_str());
	}

	return 1;
}

int GSpace::sensorEnd(GObject* radarAgent, GObject* target)
{
    RadarObject* radarObject = dynamic_cast<RadarObject*>(radarAgent);
    
	if (radarObject) {
        if(logPhysicsHandlers)
            log("%s lost %s.", radarObject->name.c_str(), target->name.c_str());
		radarObject->radarEndCollision(target);
	}
	else {
		log("sensorEnd: %s is not a radar object", radarAgent->name.c_str());
	}

    return 1;
}

//END PHYSICS

//BEGIN SENSORS

float GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
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

float GSpace::wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const
{
    return distanceFeeler(agent, feeler, GType::wall);
}

float GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return vmin(
        wallDistanceFeeler(agent, _feeler),
        distanceFeeler(agent, _feeler, GType::environment),
        distanceFeeler(agent, _feeler, GType::enemy)
    );
}

bool GSpace::feeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
    return feeler(agent,_feeler, gtype, PhysicsLayers::all);
}

bool GSpace::feeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
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
        static_cast<unsigned int>(layers),
        static_cast<unsigned int>(gtype),
        queryCallback);
    
    return collision;
}

bool GSpace::wallFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return feeler(agent, _feeler, GType::wall);
}

bool GSpace::obstacleFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return
        wallFeeler(agent, _feeler) ||
        feeler(agent, _feeler, GType::environment) ||
        feeler(agent, _feeler, GType::npc) ||
        feeler(agent, _feeler, GType::player) ||
        feeler(agent, _feeler, GType::enemy)
    ;
}

bool GSpace::lineOfSight(const GObject* agent, const GObject * target) const
{
    SpaceVect feeler_displacement = target->getPos() - agent->getPos();
    
    return !feeler(agent, feeler_displacement, GType::environment,PhysicsLayers::eyeLevel) &&
           !feeler(agent, feeler_displacement, GType::wall,PhysicsLayers::eyeLevel)
    ;
}

GObject * GSpace::pointQuery(SpaceVect pos, GType type, PhysicsLayers layers)
{
	shared_ptr<Shape> result = space.pointQueryFirst(
		pos,
		static_cast<unsigned int>(layers),
		static_cast<unsigned int>(type)
	);

	if (!result)
		return nullptr;

	return static_cast<GObject*>(result->getUserData());
}

//END SENSORS
