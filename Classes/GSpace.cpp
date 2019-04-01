//
//  GSpace.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "AreaSensor.hpp"
#include "Collectibles.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "FloorSegment.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "OverworldScene.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "replay.h"
#include "Spawner.hpp"
#include "Spell.hpp"
#include "Wall.hpp"

class RadarObject;

bool GSpace::isMultithread()
{
	return App::multithread;
}

GSpace::GSpace(GScene* gscene) : gscene(gscene)
{
	space = cpSpaceNew();
    cpSpaceSetGravity(space, cpv(0,0));
    addCollisionHandlers();

	for (type_index t : trackedTypes) {
		objByType[t] = unordered_set<GObject*>();
	}

	for (type_index t : enemyTypes) {
		objByType[t] = unordered_set<GObject*>();
	}

	controlReplay = make_unique<Replay>();
	controlReplay->scene_name = GScene::crntSceneName;
	crntState = make_unique<GState>();
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

GState* GSpace::getState() {
	if (dynamic_cast<OverworldScene*>(gscene)) {
		return App::crntState.get();
	}
	else {
		return crntState.get();
	}
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

unsigned long GSpace::getTimeUsed() const {
	return timeUsed;
}

GScene* GSpace::getScene()
{
	return gscene;
}

void GSpace::update()
{
	objectActionsMutex.lock();
	for (auto f : objectActions) {
		f();
	}
	objectActions.clear();
	objectActionsMutex.unlock();

	updateControlInfo();

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

    for(GObject* obj : updateObjects){
        obj->update();
    }
    
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

	timeUsed += _total.count();

	App::timerMutex.lock();

	App::timerSystem->addEntry(TimerType::physics, _physics);
	App::timerSystem->addEntry(TimerType::gobject, _objects);

	TimerTriplet objectUpdate = App::timerSystem->getStats(TimerType::gobject);
	TimerTriplet physicsUpdate = App::timerSystem->getStats(TimerType::physics);

	App::timerMutex.unlock();
#endif

    ++frame;
}

//BEGIN OBJECT MANIPULATION

const bool GSpace::logObjectArgs = false;

void GSpace::addDynamicLoadObject(const ValueMap& obj)
{
	string name = obj.at("name").asString();
	string type = obj.at("type").asString();

	if (name.empty()) {
		log("Un-named dynamic load object");
	}
	else if (type.empty()) {
		log("Un-typed dynamic load object");
	}
	else if (GObject::objectInfo.find(type) == GObject::objectInfo.end()) {
		log("Dynamic load object with unknown type %s.", type.c_str());
	}
	else if (dynamicLoadObjects.find(name) != dynamicLoadObjects.end()) {
		log("Dynamic load object name %s already used.", name.c_str());
	}
	else {
		dynamicLoadObjects.insert_or_assign(name, obj);
	}
}

gobject_ref GSpace::createDynamicObject(const string& name)
{
	auto it = dynamicLoadObjects.find(name);
	if (it != dynamicLoadObjects.end()) {
		return createObject(it->second);
	}
	else {
		log("Unknown dynamic load object name %s.", name.c_str());
		return nullptr;
	}
}

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

const unordered_set<GObject*>* GSpace::getObjectsByType(type_index t) const
{
	if(!isTrackedType(t)){
		log("%s is not a tracked type.", t.name());
		return nullptr;
	}

	return &(objByType.at(t));
}

bool GSpace::isTrackedType(type_index t) const
{
	return trackedTypes.find(t) != trackedTypes.end() || enemyTypes.find(t) != enemyTypes.end();
}

bool GSpace::isValid(unsigned int uuid) const
{
    return getObject(uuid) != nullptr;
}

bool GSpace::isFutureObject(ObjectIDType uuid) const
{
	return uuid != 0 && uuid > lastAddedUUID && uuid < nextObjUUID;
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

		lastAddedUUID = generator.second;

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

		if (isTrackedType(typeid(*obj))) {
			objByType[typeid(*obj)].insert(obj);
		}

		addVirtualTrack<Enemy>(obj);
		addVirtualTrack<EnemyBullet>(obj);
		addVirtualTrack<FloorSegment>(obj);
		addVirtualTrack<Wall>(obj);

        if(!obj->anonymous)
            objByName[obj->name] = obj;
        objByUUID[obj->uuid] = obj;

		if (!isNoUpdateObject(obj)) {
			updateObjects.insert(obj);
		}
        
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
	if (!obj) {
		log("GSpace::removeObject on nullptr");
		return;
	}

    //Check for object being scheduled for removal twice.
    if(find(toRemove.begin(), toRemove.end(), obj) == toRemove.end())
        toRemove.push_back(obj);
}

void GSpace::removeObject(gobject_ref ref)
{
	if (!ref.isValid()) return;

	removeObject(ref.get());
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
	updateObjects.erase(obj);

	if (isTrackedType(typeid(*obj))) {
		objByType[typeid(*obj)].erase(obj);
	}

	removeVirtualTrack<Enemy>(obj);
	removeVirtualTrack<EnemyBullet>(obj);
	removeVirtualTrack<FloorSegment>(obj);
	removeVirtualTrack<Wall>(obj);
    
	if (obj->getMass() <= 0.0 && (obj->getType() == GType::environment || obj->getType() == GType::wall)) {
		removeNavObstacle(obj->getPos(), obj->getDimensions());
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

bool GSpace::isNoUpdateObject(GObject* obj)
{
	return
		typeid(*obj) == typeid(Wall) || 
		(dynamic_cast<FloorSegment*>(obj) && typeid(*obj) != typeid(MovingPlatform))
	;
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

void GSpace::setRandomSeed(unsigned int seed)
{
	log("Random seed %u loaded.", seed);

	randomEngine.seed(seed);
	randomFloat.reset();
	randomInt.reset();

	if (!isRunningReplay) {
		log("Random seed %u saved.", seed);
		controlReplay->random_seed = seed;
	}
}

float GSpace::getRandomFloat() {
	return randomFloat(randomEngine);
}

float GSpace::getRandomFloat(float min, float max) {
	return (min + getRandomFloat() * (max - min));
}

int GSpace::getRandomInt(int min, int max) {
	return randomInt(randomEngine, boost::random::uniform_int_distribution<int>::param_type(min, max));
}

void GSpace::loadReplay(unique_ptr<Replay> replay)
{
	isRunningReplay = true;
	setRandomSeed(replay->random_seed);
	*crntState.get() = replay->crnt_state;
	controlReplay = move(replay);
}

ControlInfo GSpace::getControlInfo() const {
	return controlInfo;
}

void GSpace::setControlInfo(ControlInfo info) {
	controlInfo = info;
}

void GSpace::updateControlInfo()
{
	if (isRunningReplay)
	{
		if (frame >= controlReplay->control_data.size()) {
			addSceneAction(bind(&PlayScene::triggerReplayCompleted, getSceneAs<PlayScene>()));
		}
		else {
			controlInfo = controlReplay->getControlInfo(frame);
		}
	}
	else if(controlReplay)
	{
		controlReplay->control_data.push_back(getControlState(controlInfo));

		if (controlReplay->control_data.size() != frame + 1) {
			log("frame %ud, control replay has %ud frames.", frame, controlReplay->control_data.size());
		}
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

EnemyStatsMap GSpace::getEnemyStats()
{
	EnemyStatsMap result;
	EnemyStatsMap spawnerEnemyCount;

	unordered_set<GObject*> _objs = objByType[typeid(Spawner)];
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

void GSpace::addObjectAction(zero_arity_function f)
{
	objectActionsMutex.lock();
	objectActions.push_back(f);
	objectActionsMutex.unlock();
}

void GSpace::addSceneAction(zero_arity_function f)
{
	sceneActions.push_back(f);
}

void GSpace::createDialog(string res, bool autoAdvance)
{
	addSceneAction(
		[this, res, autoAdvance]() ->void { getScene()->createDialog(res, autoAdvance); }
	);
}

void GSpace::createDialog(string res, bool autoAdvance, zero_arity_function f)
{
	addSceneAction(
		[this, res, autoAdvance, f]() ->void { getScene()->createDialog(res, autoAdvance, f); }
	);
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

	if (!isMultiMap) {
		eraseTile(0, tile.second, layer);
	}
	else if (tile.first != -1)
	{
		eraseTile(tile.first, tile.second, layer);
	}
}

void GSpace::eraseTile(int mapID, IntVec2 pos, string layer)
{
	addSceneAction(bind(&GScene::eraseTile, gscene, mapID, pos, layer));
}

void GSpace::updatePlayerMapLocation(const SpaceVect& pos)
{
	for (size_t i = 0; i < mapAreas.size(); ++i) {
		if (mapAreas.at(i).containsPoint(pos)) {
			crntMap = i;

			if (crntChamber != ChamberID::invalid_id) {
				crntState->chamberStats.at(to_size_t(crntChamber)).roomsVisited.set(i);
			}
		}
	}

	cameraArea = calculateCameraArea(pos);

	addSceneAction(bind(&GScene::updateMapVisibility, gscene, pos));

	addSceneAction(bind(&GScene::setUnitPosition, gscene, pos));
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

	if (mapID == -1 && isMultiMap) {
		return make_pair(-1, IntVec2(0, 0));
	}

	SpaceRect map = isMultiMap ? mapAreas.at(mapID) : SpaceRect(0.0, 0.0, spaceSize.first, spaceSize.second);
	IntVec2 mapPos(floor(p.x) - map.getMinX(), map.getMaxY() - ceil(p.y));

	return make_pair(isMultiMap ? mapID : 0,mapPos);
}

int GSpace::getPlayerRoom()
{
	return crntMap;
}

void GSpace::applyMapFragment(int mapFragmentID)
{
	if (crntChamber != ChamberID::end) {
		getState()->registerMapFragment(crntChamber, mapFragmentID);

		addSceneAction(bind(&GScene::applyMapFragment, gscene, mapFragmentID));
		addHudAction(&HUD::setMapCounter, getState()->getMapFragmentCount(crntChamber));
	}
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

void GSpace::removeNavObstacle(const SpaceVect& center, const SpaceVect& boundingDimensions)
{
	for (SpaceFloat x = center.x - boundingDimensions.x / 2.0; x < center.x + boundingDimensions.x / 2.0; ++x)
	{
		for (SpaceFloat y = center.y - boundingDimensions.y / 2.0; y < center.y + boundingDimensions.y / 2.0; ++y)
		{
			unmarkObstacleTile(x, y);
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

void GSpace::unmarkObstacleTile(int x, int y)
{
	if (x >= 0 && x < spaceSize.first) {
		if (y >= 0 && y < spaceSize.second) {
			(*navMask)[y*spaceSize.first + x] = 0;
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
