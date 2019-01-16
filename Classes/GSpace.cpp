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
#include "AreaSensor.hpp"
#include "Collectibles.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Player.hpp"
#include "Spawner.hpp"
#include "Spell.hpp"
#include "TeleportPad.hpp"
#include "Upgrade.hpp"
#include "util.h"
#include "value_map.hpp"
#include "Wall.hpp"

class RadarObject;

GSpace::GSpace(GScene* gscene) : gscene(gscene)
{
    space.setGravity(SpaceVect(0,0));
    addCollisionHandlers();

	for (type_index t : trackedTypes) {
		objByType[t] = set<GObject*>();
	}

	for (type_index t : enemyTypes) {
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

	if (it != objByName.end() && warningNames.find(name) != warningNames.end()) {
		log("Warning: object name %s is not unique!", name.c_str());
	}

	return it != objByName.end() ? it->second : nullptr;
}

GObject* GSpace::getObject(unsigned int uuid) const
{
	auto it = objByUUID.find(uuid);
	return it != objByUUID.end() ? it->second : nullptr;
}

const set<GObject*>* GSpace::getObjectsByType(type_index t) const
{
	if (trackedTypes.find(t) == trackedTypes.end() && enemyTypes.find(t) == enemyTypes.end()) {
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
			warningNames.insert(obj->name);
        }

        if(objByUUID.find(obj->uuid) != objByUUID.end()){
            log("Object %s, %d UUID is not unique!", obj->name.c_str(), obj->uuid);
            delete obj;
            continue;
        }

        obj->initializeBody(*this);
        obj->initializeRadar(*this);
        obj->initializeGraphics(gscene->getSpaceLayer());
        
		if (trackedTypes.find(typeid(*obj)) != trackedTypes.end()) {
			objByType[typeid(*obj)].insert(obj);
		}
		if (enemyTypes.find(typeid(*obj)) != enemyTypes.end()) {
			objByType[typeid(*obj)].insert(obj);
		}

		if (dynamic_cast<EnemyBullet*>(obj)) {
			objByType[typeid(EnemyBullet)].insert(obj);
		}
		if (dynamic_cast<FloorSegment*>(obj)) {
			objByType[typeid(FloorSegment)].insert(obj);
		}
		if (dynamic_cast<Wall*>(obj)) {
			objByType[typeid(Wall)].insert(obj);
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

void GSpace::setBulletBodiesVisible(bool b)
{
	vector<object_ref<EnemyBullet>> bullets = getObjectsByTypeAs<EnemyBullet>();

	for (auto ref : bullets)
	{
		ref.get()->setBodyVisible(b);
	}
}

void GSpace::processRemoval(GObject* obj, bool removeSprite)
{
	obj->onRemove();

    objByName.erase(obj->name);
    objByUUID.erase(obj->uuid);

	if (trackedTypes.find(typeid(*obj)) != trackedTypes.end()) {
		objByType[typeid(*obj)].erase(obj);
	}
	if (enemyTypes.find(typeid(*obj)) != enemyTypes.end()) {
		objByType[typeid(*obj)].erase(obj);
	}

	if (dynamic_cast<EnemyBullet*>(obj)) {
		objByType[typeid(EnemyBullet)].erase(obj);
	}
	if (dynamic_cast<FloorSegment*>(obj)) {
		objByType[typeid(FloorSegment)].erase(obj);
	}
	if (dynamic_cast<Wall*>(obj)) {
		objByType[typeid(Wall)].erase(obj);
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
	if (obj->drawNode)
		obj->drawNode->removeFromParent();

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
	space.maskSeperateHandler = true;

	while (!toRemove.empty())
	{
		GObject* obj = toRemove.front();
		toRemove.pop_front();

		//Objects which will be removed this frame should have the end contact handlers called 
		//before they are deleted.
		processRemovalEndContact(obj);
		processRemoval(obj, true);
	}

	while (!toRemoveWithAnimation.empty())
	{
		auto entry = toRemoveWithAnimation.front();
		toRemoveWithAnimation.pop_front();

		Node* sprite = entry.first->sprite;

		processRemovalEndContact(entry.first);
		processRemoval(entry.first, false);

		sprite->runAction(Sequence::createWithTwoActions(entry.second, RemoveSelf::create()));
	}

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

void GSpace::setInitialObjectCount()
{
	for (auto entry : objByType)
	{
		initialObjectCount[entry.first] = entry.second.size();
	}
}

map<type_index, pair<unsigned int, unsigned int>> GSpace::getEnemyStats()
{
	map<type_index, pair<unsigned int, unsigned int>> result;

	for (type_index t : enemyTypes)
	{
		if (initialObjectCount[t] == 0) {
			continue;
		}

		result[t] = pair<unsigned int, unsigned int>(
			initialObjectCount[t] - objByType[t].size(),
			initialObjectCount[t]
		);
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

void GSpace::addWaypoint(string name, SpaceVect w)
{
	waypoints.insert_or_assign(name, w);
}

SpaceVect GSpace::getWaypoint(string name) const
{
	auto it = waypoints.find(name);

	if (it == waypoints.end()) {
		log("Unknown waypoint %s.", name.c_str());
		return SpaceVect(0, 0);
	}
	else {
		return it->second;
	}
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
	_addHandler(playerGrazeRadar, enemyBullet, playerGrazeRadarBegin,playerGrazeRadarEnd);
	_addHandlerNoEnd(playerBullet, enemy, playerBulletEnemyBegin);
	_addHandlerNoEnd(playerBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(enemyBullet, environment, bulletEnvironment);
	_addHandlerNoEnd(playerBullet, foliage, noCollide);
	_addHandlerNoEnd(enemyBullet, foliage, noCollide);
	_addHandlerNoEnd(playerBullet, enemyBullet, bulletBulletBegin);
	_addHandlerNoEnd(player, foliage, playerFlowerBegin);
    _addHandlerNoEnd(player,collectible,playerCollectibleBegin);
	_addHandlerNoEnd(player, upgrade, playerUpgradeBegin);
    _addHandlerNoEnd(player,npc,collide);
	_addHandlerNoEnd(playerBullet, wall, bulletWall);
	_addHandlerNoEnd(enemyBullet, wall, bulletWall);    

	_addHandler(playerSensor, player, sensorStart, sensorEnd);
	_addHandler(playerSensor, playerBullet, sensorStart, sensorEnd);

	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, collectible, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);

	_addHandler(teleportPad, player, teleportPadObjectBegin, teleportPadObjectEnd);
	_addHandler(teleportPad, enemy, teleportPadObjectBegin, teleportPadObjectEnd);
	_addHandler(teleportPad, environment, teleportPadObjectBegin, teleportPadObjectEnd);

	_addHandler(spawner, player, spawnerObjectBegin, spawnerObjectEnd);
	_addHandler(spawner, enemy, spawnerObjectBegin, spawnerObjectEnd);
	_addHandler(spawner, environment, spawnerObjectBegin, spawnerObjectEnd);

	_addHandler(player, effectArea, agentEffectAreaBegin, agentEffectAreaEnd);
	_addHandler(enemy, effectArea, agentEffectAreaBegin, agentEffectAreaEnd);

	_addHandler(player, areaSensor, playerAreaSensorBegin, playerAreaSensorEnd);
	_addHandler(enemy, areaSensor, enemyAreaSensorBegin, enemyAreaSensorEnd);
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

int GSpace::playerGrazeRadarBegin(GObject* playerRadar, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeTouch(_bullet);
	}

	return 1;
}

int GSpace::playerGrazeRadarEnd(GObject* playerRadar, GObject* bullet)
{
	Player* player = dynamic_cast<Player*>(playerRadar);
	EnemyBullet* _bullet = dynamic_cast<EnemyBullet*>(bullet);

	if (player && _bullet) {
		player->onGrazeCleared(_bullet);
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

int GSpace::bulletBulletBegin(GObject* a, GObject* b)
{
	Bullet* _a = dynamic_cast<Bullet*>(a);
	Bullet* _b = dynamic_cast<Bullet*>(b);

	if (_a && _b) {
		_a->onBulletCollide(_b);
		_b->onBulletCollide(_a);
	}

	return 0;
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

int GSpace::playerUpgradeBegin(GObject* a, GObject* b)
{
	Player* p = dynamic_cast<Player*>(a);
	Upgrade* u = dynamic_cast<Upgrade*>(b);

	if (p && u) {
		p->applyUpgrade(u);
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

int GSpace::teleportPadObjectBegin(GObject* teleportPad, GObject* obj)
{
	TeleportPad* _tp = dynamic_cast<TeleportPad*>(teleportPad);

	if (_tp && obj) {
		_tp->onContact(obj);
	}

	return 1;
}

int GSpace::teleportPadObjectEnd(GObject* teleportPad, GObject* obj)
{
	TeleportPad* _tp = dynamic_cast<TeleportPad*>(teleportPad);

	if (_tp && obj) {
		_tp->onEndContact(obj);
	}

	return 1;
}

int GSpace::spawnerObjectBegin(GObject* spawner, GObject* obj)
{
	Spawner* _s = dynamic_cast<Spawner*>(spawner);

	if (_s && obj) {
		_s->onContact(obj);
	}

	return 1;
}

int GSpace::spawnerObjectEnd(GObject* spawner, GObject* obj)
{
	Spawner* _s = dynamic_cast<Spawner*>(spawner);

	if (_s && obj) {
		_s->onEndContact(obj);
	}

	return 1;
}

int GSpace::playerAreaSensorBegin(GObject* a, GObject *b)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerContact(p);
	}
	return 1;
}

int GSpace::playerAreaSensorEnd(GObject* a, GObject *b)
{
	Player* p = dynamic_cast<Player*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (p && as) {
		as->onPlayerEndContact(p);
	}
	return 1;
}

int GSpace::enemyAreaSensorBegin(GObject* a, GObject *b)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyContact(e);
	}
	return 1;
}

int GSpace::enemyAreaSensorEnd(GObject* a, GObject *b)
{
	Enemy* e = dynamic_cast<Enemy*>(a);
	AreaSensor* as = dynamic_cast<AreaSensor*>(b);

	if (e && as) {
		as->onEnemyEndContact(e);
	}
	return 1;
}

//END PHYSICS

//BEGIN SENSORS

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
	return distanceFeeler(agent, _feeler, gtype, PhysicsLayers::all);
}

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
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
        static_cast<unsigned int>(layers),
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
		trapFloorDistanceFeeler(agent,_feeler),
        wallDistanceFeeler(agent, _feeler),
        distanceFeeler(agent, _feeler, GType::environment),
        distanceFeeler(agent, _feeler, GType::enemy)
    );
}

SpaceFloat GSpace::trapFloorDistanceFeeler(const GObject* agent, SpaceVect feeler) const
{
	return distanceFeeler(agent, feeler, GType::floorSegment, PhysicsLayers::belowFloor);
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

GObject* GSpace::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;

	GObject* bestResult = nullptr;
	SpaceFloat bestDistance = feeler.length();

	auto queryCallback = [agent, &bestResult, &bestDistance](std::shared_ptr<Shape> shape, cp::Float distance, cp::Vect vect) -> void {

		GObject* _crntObj = static_cast<GObject*>(shape->getUserData());

		if (_crntObj && _crntObj != agent && distance < bestDistance) {
			bestResult = _crntObj;
			bestDistance = distance;
		}
	};

	space.segmentQuery(
		start,
		end,
		static_cast<unsigned int>(layers),
		static_cast<unsigned int>(gtype),
		queryCallback);

	return bestResult;
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

InteractibleObject* GSpace::interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const
{
	array<GObject*, 3> objects;
	objects[0] = objectFeeler(agent, feeler, GType::npc, PhysicsLayers::all);
	objects[1] = objectFeeler(agent, feeler, GType::enemy, PhysicsLayers::all);
	objects[2] = objectFeeler(agent, feeler, GType::environment, PhysicsLayers::all);

	for (GObject* obj : objects) {
		InteractibleObject* _interact = dynamic_cast<InteractibleObject*>(obj);

		if (_interact) {
			return _interact;
		}
	}

	return nullptr;
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

bool GSpace::rectangleQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers)
{
	bool collision = false;

	auto queryCallback = [&collision,center,dimensions](std::shared_ptr<Shape> shape) -> void {
		GObject* obj = static_cast<GObject*>(shape->getUserData());

		if (obj) {
			SpaceVect pos = obj->getPos();

			//Physics engine registers collisions with objects that aren't even close to the query area.
			if (pos.x >= center.x - dimensions.x / 2 && pos.x <= center.x + dimensions.x / 2 &&
				pos.y >= center.y - dimensions.y / 2 && pos.y <= center.y + dimensions.y / 2) {
				collision = true;
			}
		}
	};

	shared_ptr<Body> _body = space.makeStaticBody();
	_body->setPos(center);

	shared_ptr<PolyShape> area = PolyShape::rectangle(_body, dimensions);
	area->setBody(_body);
	_body->addShape(area);

	setShapeProperties(area, layers, type, true);

	space.shapeQuery(
		area,
		queryCallback
	);

	return collision;
}

//END SENSORS
