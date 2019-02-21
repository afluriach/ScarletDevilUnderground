//
//  GSpace.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "Collectibles.hpp"
#include "Door.hpp"
#include "EnemyBullet.hpp"
#include "FloorSegment.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "Spawner.hpp"
#include "Wall.hpp"

class RadarObject;

GSpace::GSpace(GScene* gscene) : gscene(gscene)
{
	space = cpSpaceNew();
    cpSpaceSetGravity(space, cpv(0,0));
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

	cpSpaceFree(space);
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
	cpSpaceStep(space, App::secondsPerFrame);
    
#if USE_TIMERS
	chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
#endif

    for(GObject* obj: objByUUID | boost::adaptors::map_values){
        obj->update();
    }

	objectActionsMutex.lock();
	for (auto f : objectActions) {
		f();
	}
	objectActions.clear();
	objectActionsMutex.unlock();
    
    processRemovals();
    
    //process additions
    processAdditions();
    
	gscene->addActions(sceneActions);
	sceneActions.clear();

#if USE_TIMERS
	chrono::steady_clock::time_point t4 = chrono::steady_clock::now();

	chrono::duration<long, micro> _physics = chrono::duration_cast<chrono::microseconds>(t3 - t2);
	chrono::duration<long, micro> _total = chrono::duration_cast<chrono::microseconds>(t4 - t1);
	chrono::duration<long, micro> _objects = _total - _physics;

	App::timerSystem->addEntry(TimerType::physics, _physics);
	App::timerSystem->addEntry(TimerType::gobject, _objects);

	PlayScene* ps = dynamic_cast<PlayScene*>(gscene);

	if (ps && frame % 60 == 0) {
		addSceneAction(
			[ps]()->void {
				ps->hud->setPerformanceStats(
					App::timerSystem->getStats(TimerType::gobject),
					App::timerSystem->getStats(TimerType::physics)
				);
			},
			GScene::updateOrder::hudUpdate
		);
	}

#endif

    ++frame;
}

//BEGIN OBJECT MANIPULATION

const bool GSpace::logObjectArgs = false;

gobject_ref GSpace::createObject(const ValueMap& obj)
{
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

void GSpace::addWallBlock(const SpaceVect& ll, const SpaceVect& ur)
{
	addWallBlock(SpaceRect(ll.x, ll.y, ur.x - ll.x, ur.y - ll.y));
}

void GSpace::addWallBlock(const SpaceRect& area)
{
	createObject(
		GObject::make_object_factory<Wall>(area.center, area.dimensions)
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
        obj->initializeGraphics();
        
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

void GSpace::removeObjectWithAnimation(GObject* obj, ActionGeneratorType action)
{
	toRemoveWithAnimation.push_back(pair<GObject*, ActionGeneratorType>(obj,action));
}

void GSpace::setBulletBodiesVisible(bool b)
{
	vector<object_ref<EnemyBullet>> bullets = getObjectsByTypeAs<EnemyBullet>();

	for (auto ref : bullets)
	{
		ref.get()->setBodyVisible(b);
	}
}

void GSpace::processRemoval(GObject* obj, bool _removeSprite)
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
    
	if (obj->radarShape) {
		cpSpaceRemoveShape(space, obj->radarShape);
		cpShapeFree(obj->radarShape);
	}
	if (obj->radar) {
		cpSpaceRemoveBody(space, obj->radar);
		cpBodyFree(obj->radar);
	}

	if (obj->bodyShape) {
		cpSpaceRemoveShape(space, obj->bodyShape);
		cpShapeFree(obj->bodyShape);
	}
	if (obj->body) {
		if (obj->getMass() > 0.0) {
			cpSpaceRemoveBody(space, obj->body);
		}
		cpBodyFree(obj->body);
	}

	if (obj->crntSpell.get()) {
		obj->crntSpell.get()->end();
	}

	if (_removeSprite && obj->spriteID != 0) {
		removeSprite(obj->spriteID);
	}
	if (_removeSprite && obj->drawNodeID != 0) {
		removeSprite(obj->drawNodeID);
	}
	if (_removeSprite && obj->lightID != 0) {
		removeLightSource(obj->lightID);
	}

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
	while (!toRemove.empty())
	{
		GObject* obj = toRemove.front();
		toRemove.pop_front();

		processRemoval(obj, true);
	}

	while (!toRemoveWithAnimation.empty())
	{
		auto entry = toRemoveWithAnimation.front();
		toRemoveWithAnimation.pop_front();

		unsigned int spriteID = entry.first->spriteID;

		processRemoval(entry.first, false);

		gscene->removeSpriteWithAnimation(spriteID, entry.second);
	}
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
	map<type_index, pair<unsigned int, unsigned int>> spawnerEnemyCount;

	set<GObject*> _objs = objByType[typeid(Spawner)];
	for (GObject* _obj : _objs)
	{
		Spawner* s = dynamic_cast<Spawner*>(_obj);
		type_index t = s->getSpawnType();
		
		auto it = spawnerEnemyCount.find(t);
		if (it == spawnerEnemyCount.end()) {
			spawnerEnemyCount.insert_or_assign(t, pair<unsigned int, unsigned int>(0, 0));
		}

		spawnerEnemyCount.at(t).first += s->getSpawnCount();
		spawnerEnemyCount.at(t).second += s->getSpawnLimit();
	}

	for (type_index t : enemyTypes)
	{
		if (initialObjectCount[t] == 0 && spawnerEnemyCount[t].second == 0) {
			continue;
		}

		result[t] = pair<unsigned int, unsigned int>(
			initialObjectCount[t] + spawnerEnemyCount[t].first - objByType[t].size(),
			initialObjectCount[t] + spawnerEnemyCount[t].second
		);
	}

	return result;
}

void GSpace::addObjectAction(function<void()> f)
{
	objectActionsMutex.lock();
	objectActions.push_back(f);
	objectActionsMutex.unlock();
}

void GSpace::addSceneAction(pair<function<void(void)>, GScene::updateOrder> entry)
{
	sceneActions.push_back(entry);
}

void GSpace::addSceneAction(function<void(void)> f, GScene::updateOrder order)
{
	sceneActions.push_back(make_pair(f,order));
}

void GSpace::teleportPlayerToDoor(string doorName)
{
	Door* d = getObjectAs<Door>(doorName);
	Player* p = getObjectAs<Player>("player");

	if (d && p) {
		p->moveToDestinationDoor(d);
	}
}

void GSpace::setSuppressAction(bool b)
{
	suppressAction = b;
}

bool GSpace::getSuppressAction()
{
	return suppressAction;
}

void GSpace::eraseTile(const SpaceVect& p, string layer)
{
	pair<int, IntVec2> tile = getTilePosition(p);

	if (tile.first != -1)
	{
		eraseTile(tile.first, tile.second, layer);
	}
}

void GSpace::eraseTile(int mapID, IntVec2 pos, string layer)
{
	addSceneAction(
		bind(&GScene::eraseTile, gscene, mapID, pos, layer),
		GScene::updateOrder::sceneUpdate
	);
}

void GSpace::updatePlayerMapLocation(const SpaceVect& pos)
{
	for (int i = 0; i < mapAreas.size(); ++i) {
		if (mapAreas.at(i).containsPoint(pos)) {
			crntMap = i;
		}
	}

	cameraArea = calculateCameraArea(pos);

	addSceneAction(
		bind(&GScene::updateMapVisibility, gscene, pos),
		GScene::updateOrder::sceneUpdate
	);

	addSceneAction(
		bind(&GScene::setUnitPosition, gscene, pos),
		GScene::updateOrder::moveCamera
	);
}

void GSpace::addMapArea(const SpaceRect& area)
{
	mapAreas.push_back(area);
}

SpaceRect GSpace::getCameraArea()
{
	return cameraArea;
}

const vector<SpaceRect>& GSpace::getMapAreas()
{
	return mapAreas;
}

int GSpace::getMapLocation(SpaceRect r)
{
	return getAreaIndex(mapAreas, r);
}

bool GSpace::isInCameraArea(SpaceRect r)
{
	return cameraArea.intersectsRect(r);
}

bool GSpace::isInPlayerRoom(SpaceVect v)
{
	return isInArea(mapAreas, v, crntMap);
}

pair<int, IntVec2> GSpace::getTilePosition(SpaceVect p)
{
	int mapID = getAreaIndex(mapAreas, p);

	if (mapID == -1) {
		return make_pair(-1, IntVec2(0, 0));
	}

	SpaceRect map = mapAreas.at(mapID);
	IntVec2 mapPos(floor(p.x) - map.getMinX(), map.getMaxY() - ceil(p.y));

	return make_pair(mapID,mapPos);
}

int GSpace::getPlayerRoom()
{
	return crntMap;
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
	return dynamic_cast<FloorSegment*>(pointQuery(pos, GType::floorSegment, PhysicsLayers::belowFloor));
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

	if (tileCoords.size() < 2) {
		return result;
	}

	result.push_back(toChipmunkWithCentering(tileCoords[0]));
	result.push_back(toChipmunkWithCentering(tileCoords[1]));
	SpaceVect direction = toChipmunkWithCentering(tileCoords[1]) - toChipmunkWithCentering(tileCoords[0]);

	for_irange(i, 2, tileCoords.size())
	{
		SpaceVect crntTile = toChipmunkWithCentering(tileCoords[i]);
		SpaceVect crntDirection = crntTile - result.back();

		if (SpaceVect::fuzzyMatch(direction, crntDirection)){
			result.back() = crntTile;
		}
		else {
			result.push_back(crntTile);
			direction = crntDirection;
		}
	}

	SpaceVect endPoint = toChipmunkWithCentering(tileCoords.back());

	//Endpoint will not be added loop unless it happens to be a turn (i.e. not colinear with the previous tile coord)
	if (!SpaceVect::fuzzyMatch(result.back(), endPoint)) {
		result.push_back(endPoint);
	}

	return result;
}

//END NAVIGATION
