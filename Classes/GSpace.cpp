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
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Player.hpp"
#include "Spell.hpp"
#include "util.h"
#include "value_map.hpp"
#include "Wall.hpp"

class RadarObject;

GSpace::GSpace(SpaceLayer* spaceLayer, GScene* gscene) : spaceLayer(spaceLayer), gscene(gscene)
{
    space.setGravity(SpaceVect(0,0));
    addCollisionHandlers();

	for (type_index t : trackedTypes) {
		objByType[t] = set<GObject*>();
	}
}

GSpace::~GSpace()
{
    //Avoid calling seperation (or "end contact") handlers.
    cp::Space::maskSeperateHandler = true;

    //Process removal modified objByUUID.
    vector<GObject*> objs;

	for (GObject* obj : objByUUID | boost::adaptors::map_values) {
		objs.push_back(obj);
	}
        
    for(GObject* obj: objs){
        processRemoval(obj, true);
    }
    
    if(navMask)
        delete navMask;

    cp::Space::maskSeperateHandler = false;
}

IntVec2 GSpace::getSize() const {
	return spaceSize;
}

void GSpace::setSize(int x, int y)
{
    spaceSize = IntVec2(x,y);
    if(navMask)
        delete navMask;
    navMask = new boost::dynamic_bitset<>(x*y);
}

unsigned int GSpace::getFrame() const{
	return frame;
}

GScene* GSpace::getScene()
{
	return gscene;
}

void GSpace::update()
{
#if USE_TIMERS
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
#endif

    //Run inits for recently added objects
    initObjects();
    
#if USE_TIMERS
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
#endif

    //physics step
    space.step(App::secondsPerFrame);
    
#if USE_TIMERS
	chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
#endif

    for(GObject* obj: objByUUID | boost::adaptors::map_values){
        obj->update();
    }

	for(GObject* obj: objByUUID | boost::adaptors::map_values) {
		obj->applyPhysicsProperties();
	}
    
    processRemovals();
    
    //process additions
    processAdditions();
    
#if USE_TIMERS
	chrono::steady_clock::time_point t4 = chrono::steady_clock::now();

	chrono::duration<long, micro> _physics = chrono::duration_cast<chrono::microseconds>(t3 - t2);
	chrono::duration<long, micro> _total = chrono::duration_cast<chrono::microseconds>(t4 - t1);
	chrono::duration<long, micro> _objects = _total - _physics;

	App::timerSystem->addEntry(TimerType::physics, _physics);
	App::timerSystem->addEntry(TimerType::gobject, _objects);

#endif

    ++frame;
}

//BEGIN OBJECT MANIPULATION

const bool GSpace::logObjectArgs = false;

gobject_ref GSpace::createObject(const ValueMap& obj)
{
    if(logObjectArgs)
        printValueMap(obj);

    string type = obj.at("type").asString();
	
	return createObject(GObject::factoryMethodByType(type, obj));
}

gobject_ref GSpace::createObject(ObjectGeneratorType generator)
{
	ObjectIDType id = getAndIncrementObjectUUID();

    toAdd.push_back(make_pair(generator,id));

	return gobject_ref(this,id);
}

void GSpace::createObjects(const ValueVector& objs)
{
    for(const Value& obj: objs)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        createObject(objAsMap);
    }
}

void GSpace::addWallBlock(SpaceVect ll,SpaceVect ur)
{
    SpaceVect center = (ll + ur) / 2;
    SpaceVect dim = ur - ll;
    
	createObject(
		GObject::make_object_factory<Wall>(center, dim)
	);
}

gobject_ref GSpace::getObjectRef(const string& name) const
{
    auto it = objByName.find(name);
    return it != objByName.end() ? it->second : nullptr;
}

gobject_ref GSpace::getObjectRef(unsigned int uuid) const
{
    auto it = objByUUID.find(uuid);
    return it != objByUUID.end() ? it->second : nullptr;
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

const set<GObject*>* GSpace::getObjecstByType(type_index t) const
{
	if (trackedTypes.find(t) == trackedTypes.end()) {
		log("%s is not a tracked type.", t.name());
		return nullptr;
	}

	return &(objByType.at(t));
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
    for(const generator_pair& generator: toAdd)
    {
		GObject* obj = generator.first(this, generator.second);

		if (!obj)
			continue;

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
        obj->initializeGraphics(spaceLayer);
        
		if (trackedTypes.find(typeid(*obj)) != trackedTypes.end()) {
			objByType[typeid(*obj)].insert(obj);
		}

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
	obj->onRemove();

    objByName.erase(obj->name);
    objByUUID.erase(obj->uuid);

	if (trackedTypes.find(typeid(*obj)) != trackedTypes.end()) {
		objByType[typeid(*obj)].erase(obj);
	}

	currentContacts.erase(obj);
    
	for (object_ref<FloorSegment> fs_ref : obj->crntFloorContacts) {
		FloorSegment* fs = fs_ref.get();
		if (fs) {
			fs->onEndContact(obj);
		}
	}

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
    for(GObject* obj: addedLastFrame)
    {
        obj->init();
    }
    addedLastFrame.clear();
}

void GSpace::processRemovals()
{
	//Objects which will be removed this frame should have the end contact handlers called 
	//before they are deleted.
	for(GObject* obj: toRemove) {
		processRemovalEndContact(obj);
	}
	
	for (GObject *obj : toRemoveWithAnimation | boost::adaptors::map_keys) {
		processRemovalEndContact(obj);
	}

	space.maskSeperateHandler = true;

	for(GObject* obj: toRemove){
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

unsigned int GSpace::getAndIncrementObjectUUID()
{
	return nextObjUUID++;
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
	FloorSegment* result = nullptr;

	result = dynamic_cast<FloorSegment*>(pointQuery(pos, GType::floorSegment, PhysicsLayers::floor));

	if (!result)
		result = dynamic_cast<FloorSegment*>(pointQuery(pos, GType::floorSegment, PhysicsLayers::belowFloor));

	return result;
}

void GSpace::addNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions)
{
    for(SpaceFloat x = center.x - boundingDimensions.x/2.0; x < center.x + boundingDimensions.x/2.0; ++x)
    {
        for(SpaceFloat y = center.y - boundingDimensions.y/2.0; y < center.y + boundingDimensions.y/2.0; ++y)
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

vector<SpaceVect> GSpace::pathToTile(IntVec2 begin, IntVec2 end)
{
	vector<SpaceVect> result;

	vector<pair<int, int>> tileCoords = graph::gridAStar(
		*navMask,
		begin,
		end,
		getSize()
	);

	//Convert to center position
	for(auto const& tile: tileCoords) {
		result.push_back(SpaceVect(tile.first + 0.5, tile.second + 0.5));
	}

	return result;
}

//END NAVIGATION

//BEGIN PHYSICS
#define _addHandler(a,b,begin,end) AddHandler<GType::a, GType::b>(&GSpace::begin,&GSpace::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler<GType::a, GType::b>(&GSpace::begin,nullptr)

void GSpace::addCollisionHandlers()
{
	_addHandler(player, enemy, playerEnemyBegin, playerEnemyEnd);
	_addHandlerNoEnd(player, enemyBullet, playerEnemyBulletBegin);
	_addHandler(player, enemyBulletRadar, playerEnemyBulletRadarBegin,playerEnemyBulletRadarEnd);
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

	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, collectible, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);

	_addHandler(player, effectArea, agentEffectAreaBegin, agentEffectAreaEnd);
	_addHandler(enemy, effectArea, agentEffectAreaBegin, agentEffectAreaEnd);
}

const set<GType> GSpace::selfCollideTypes = {
	GType::enemy,
	GType::environment,
	GType::npc
};

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
    SpaceFloat radius,
    SpaceFloat mass,
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
            addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
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
    SpaceFloat mass,
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

	for(const contact& c: contactList) {
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
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

    if(logPhysicsHandlers)
        log("%s hit by %s", player->name.c_str(), bullet->name.c_str());

	if (player && _bullet) {
		_bullet->onAgentCollide(player);
		_bullet->invalidateGraze();
	}

    return 1;
}

int GSpace::playerEnemyBulletRadarBegin(GObject* playerObj, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerObj);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		_bullet->onGrazeTouch(player);
	}

	return 1;
}

int GSpace::playerEnemyBulletRadarEnd(GObject* playerObj, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerObj);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		_bullet->onGrazeCleared (player);
	}

	return 1;
}

int GSpace::playerBulletEnemyBegin(GObject* a, GObject* b)
{    
    Bullet* bullet = dynamic_cast<Bullet*>(a);
    Agent* _enemy_agent = dynamic_cast<Agent*>(b);
    
    if(!bullet)
        log("%s is not a Bullet", a->getName().c_str());
    if(!_enemy_agent)
        log("%s is not an Enemy", b->getName().c_str());
    
	if (bullet && _enemy_agent){
		bullet->onAgentCollide(_enemy_agent);
	}

    if(logPhysicsHandlers)
        log("%s hit by %s", b->name.c_str(), a->name.c_str());
    
    return 1;
}

int GSpace::agentEffectAreaBegin(GObject* a, GObject* b)
{
	Agent* agent = dynamic_cast<Agent*>(a);
	EffectArea* area = dynamic_cast<EffectArea*>(b);

	if (agent && area) {
		area->onContact(agent);
	}

	return 1;
}
int GSpace::agentEffectAreaEnd(GObject* a, GObject* b)
{
	Agent* agent = dynamic_cast<Agent*>(a);
	EffectArea* area = dynamic_cast<EffectArea*>(b);

	if (agent && area) {
		area->onEndContact(agent);
	}

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

int GSpace::bulletEnvironment(GObject* bullet, GObject* environment)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);

	if (_b && environment) {
		_b->onEnvironmentCollide(environment);
	}
    
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

int GSpace::bulletWall(GObject* bullet, GObject* wall)
{
	Bullet* _b = dynamic_cast<Bullet*>(bullet);
	Wall* _w = dynamic_cast<Wall*>(wall);

	if (_b && _w) {
		_b->onWallCollide(_w);
	}

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

int GSpace::floorObjectBegin(GObject* floorSegment, GObject* obj)
{
	FloorSegment* fs = dynamic_cast<FloorSegment*>(floorSegment);

	if (dynamic_cast<FloorSegment*>(obj)) {
		log("GSpace::floorObjectBegin: FloorSegment should not collide with another one.");
		return 0;
	}

	else if (!fs || !obj) {
		return 0;
	}

	else{
		obj->message<GObject>(obj, &GObject::onContactFloorSegment, object_ref<FloorSegment>(fs));
		return 1;
	}
}

int GSpace::floorObjectEnd(GObject* floorSegment, GObject* obj)
{
	FloorSegment* fs = dynamic_cast<FloorSegment*>(floorSegment);

	if (dynamic_cast<FloorSegment*>(obj)) {
		log("GSpace::floorObjectEnd: FloorSegment should not collide with another one.");
		return 0;
	}

	else if (!fs || !obj) {
		return 0;
	}

	else {
		obj->message<GObject>(obj, &GObject::onEndContactFloorSegment, object_ref<FloorSegment>(fs));
		return 1;
	}
}


//END PHYSICS

//BEGIN SENSORS

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
    
    //Distance along the segment is scaled [0,1].
    SpaceFloat closest = 1.0;
    
    auto queryCallback = [&closest,agent] (std::shared_ptr<Shape> shape, cp::Float distance, cp::Vect vect) -> void {
        
        if(shape->getUserData() != agent){
            closest = min<SpaceFloat>(closest, distance);
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

SpaceFloat GSpace::wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const
{
    return distanceFeeler(agent, feeler, GType::wall);
}

SpaceFloat GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect _feeler) const
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
