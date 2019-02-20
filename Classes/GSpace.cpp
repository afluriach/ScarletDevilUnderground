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
#include "Door.hpp"
#include "EffectArea.hpp"
#include "Enemy.hpp"
#include "EnemyBullet.hpp"
#include "enum.h"
#include "FloorSegment.hpp"
#include "GObject.hpp"
#include "Graph.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "Player.hpp"
#include "PlayScene.hpp"
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

//BEGIN GRAPHICS

LightID GSpace::addLightSource(CircleLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

LightID GSpace::addLightSource(AmbientLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

LightID GSpace::addLightSource(ConeLightArea light)
{
	LightID id = gscene->getLightID();

	addLightmapAction([this, id, light]()->void {
		gscene->addLightSource(id, light);
	});
	return id;
}

void GSpace::updateLightSource(LightID id, ConeLightArea light)
{
	addLightmapAction([this, id, light]()->void {
		gscene->updateLightSource(id, light);
	});
}

void GSpace::removeLightSource(LightID id)
{
	addLightmapAction([this, id]()->void {
		gscene->removeLightSource(id);
	});
}

void GSpace::setLightSourcePosition(LightID id, SpaceVect pos)
{
	addLightmapAction([this, id, pos]()->void {
		gscene->setLightSourcePosition(id, pos);
	});
}

SpriteID GSpace::createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, path, sceneLayer, pos, zoom]() -> void {
		gscene->createSprite(id, path, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, name, frameCount, duration, sceneLayer, pos, zoom]() -> void {
		gscene->createLoopAnimation(id, name, frameCount, duration, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, sceneLayer, pos, zoom]() -> void {
		gscene->createDrawNode(id, sceneLayer, pos, zoom);
	});

	return id;
}

SpriteID GSpace::createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	SpriteID id = gscene->getSpriteID();

	addSpriteAction([this, id, path, isAgentAnimation, sceneLayer, pos, zoom]() -> void {
		gscene->createAgentSprite(id, path, isAgentAnimation, sceneLayer, pos, zoom);
	});

	return id;
}

void GSpace::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	addSpriteAction([this, id, path, isAgentAnimation]() -> void {
		gscene->loadAgentAnimation(id, path, isAgentAnimation);
	});
}

void GSpace::setAgentAnimationDirection(SpriteID id, Direction d)
{
	addSpriteAction([this, id, d]() -> void {
		gscene->setAgentAnimationDirection(id, d);
	});
}

void GSpace::setAgentAnimationFrame(SpriteID id, int frame)
{
	addSpriteAction([this, id, frame]() -> void {
		gscene->setAgentAnimationFrame(id, frame);
	});
}

void GSpace::clearDrawNode(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->clearDrawNode(id);
	});
}

void GSpace::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	addSpriteAction([this, id, lowerLeft, upperRight, color]() -> void {
		gscene->drawSolidRect(id, lowerLeft, upperRight, color);
	});
}

void GSpace::drawSolidCone(SpriteID id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color)
{
	addSpriteAction([this, id, center, radius, startAngle, endAngle, segments, color]() -> void {
		gscene->drawSolidCone(id, center, radius, startAngle, endAngle, segments, color);
	});
}

void GSpace::drawSolidCircle(SpriteID id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
	addSpriteAction([this, id, center, radius, angle, segments, color]() -> void {
		gscene->drawSolidCircle(id, center, radius, angle, segments, color);
	});
}

void GSpace::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction([this, id, generator]() -> void {
		gscene->runSpriteAction(id, generator);
	});
}

void GSpace::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	addSpriteAction([this, id, action]() -> void {
		gscene->stopSpriteAction(id, action);
	});
}

void GSpace::stopAllSpriteActions(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->stopAllSpriteActions(id);
	});
}

void GSpace::removeSprite(SpriteID id)
{
	addSpriteAction([this, id]() -> void {
		gscene->removeSprite(id);
	});
}

void GSpace::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	addSpriteAction([this, id, generator]() -> void {
		gscene->removeSpriteWithAnimation(id, generator);
	});
}

void GSpace::setSpriteVisible(SpriteID id, bool val)
{
	addSpriteAction([this, id, val]() -> void {
		gscene->setSpriteVisible(id, val);
	});
}

void GSpace::setSpriteOpacity(SpriteID id, unsigned char op)
{
	addSpriteAction([this, id, op]() -> void {
		gscene->setSpriteOpacity(id, op);
	});
}

void GSpace::setSpriteTexture(SpriteID id, string path)
{
	addSpriteAction([this, id, path]()->void {
		gscene->setSpriteTexture(id, path);
	});
}

void GSpace::setSpriteAngle(SpriteID id, float cocosAngle)
{
	addSpriteAction([this, id, cocosAngle]() -> void {
		gscene->setSpriteAngle(id, cocosAngle);
	});
}

void GSpace::setSpritePosition(SpriteID id, Vec2 pos)
{
	addSpriteAction([this, id, pos]() -> void {
		gscene->setSpritePosition(id, pos);
	});
}

void GSpace::setSpriteZoom(SpriteID id, float zoom)
{
	addSpriteAction([this, id, zoom]() -> void {
		gscene->setSpriteZoom(id, zoom);
	});
}

void GSpace::setSpriteColor(SpriteID id, Color3B color)
{
	addSpriteAction([this, id, color]()->void {
		gscene->setSpriteColor(id, color);
	});
}

void GSpace::clearSubroomMask(unsigned int roomID)
{
	addSpriteAction([this, roomID]() -> void {
		gscene->clearSubroomMask(roomID);
	});
}

void GSpace::addLightmapAction(function<void()> f)
{
	sceneActions.push_back(make_pair(f, GScene::updateOrder::lightmapUpdate));
}

void GSpace::addSpriteAction(function<void()> f)
{
	sceneActions.push_back(make_pair(f, GScene::updateOrder::spriteUpdate));
}

//END GRAPHICS

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

//BEGIN PHYSICS
#define _addHandler(a,b,begin,end) AddHandler<GType::a, GType::b>(&GSpace::begin,&GSpace::end)
#define _addHandlerNoEnd(a,b,begin) AddHandler<GType::a, GType::b>(&GSpace::begin,nullptr)

int GSpace::beginContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB

		GSpace* _this = static_cast<GSpace*>(data);

	GType typeA = static_cast<GType>(arb->a_private->collision_type);
	GType typeB = static_cast<GType>(arb->b_private->collision_type);

	auto it = _this->beginContactHandlers.find(collision_type(typeA, typeB));

	if (it == _this->beginContactHandlers.end()) {
		it = _this->beginContactHandlers.find(collision_type(typeB, typeA));
		swap(a, b);
	}

	//No collide;
	if (it == _this->beginContactHandlers.end())
		return 0;

	if (a && b && it->second) {
		int(GSpace::*begin_method)(GObject*, GObject*) = it->second;
		(_this->*begin_method)(a, b);
	}

	return 1;
}

void GSpace::endContact(cpArbiter* arb, cpSpace* space, void* data)
{
	OBJS_FROM_ARB

		GSpace* _this = static_cast<GSpace*>(data);

	GType typeA = static_cast<GType>(arb->a_private->collision_type);
	GType typeB = static_cast<GType>(arb->b_private->collision_type);

	auto it = _this->endContactHandlers.find(collision_type(typeA, typeB));

	if (it == _this->endContactHandlers.end()) {
		it = _this->endContactHandlers.find(collision_type(typeB, typeA));
		swap(a, b);
	}

	if (it == _this->endContactHandlers.end())
		return;

	if (a && b && it->second) {
		int(GSpace::*end_method)(GObject*, GObject*) = it->second;
		(_this->*end_method)(a, b);
	}
}


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

	_addHandlerNoEnd(enemy, enemy, collide);
	_addHandlerNoEnd(environment, environment, collide);
	_addHandlerNoEnd(npc, npc, collide);

	_addHandler(enemySensor, player, sensorStart, sensorEnd);
	_addHandler(enemySensor, playerBullet, sensorStart, sensorEnd);
	_addHandler(enemySensor, enemy, sensorStart, sensorEnd);

	_addHandler(floorSegment, player, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, enemy, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, npc, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, collectible, floorObjectBegin, floorObjectEnd);
	_addHandler(floorSegment, environment, floorObjectBegin, floorObjectEnd);

	_addHandler(player, areaSensor, playerAreaSensorBegin, playerAreaSensorEnd);
	_addHandler(enemy, areaSensor, enemyAreaSensorBegin, enemyAreaSensorEnd);
	_addHandler(environment, areaSensor, environmentAreaSensorBegin, environmentAreaSensorEnd);
}

const bool GSpace::logBodyCreation = false;
const bool GSpace::logPhysicsHandlers = false;

void setShapeProperties(cpShape* shape, PhysicsLayers layers, GType type, bool sensor)
{
    shape->layers = to_uint(layers);
    shape->group = 0;
	shape->collision_type = to_uint(type);
	shape->sensor = sensor;
}

pair<cpShape*, cpBody*> GSpace::createCircleBody(
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
    
	cpBody* body;
	cpShape* shape;

    if(mass <= 0.0){
        body = cpBodyNewStatic();
        if(type == GType::environment || type == GType::wall)
            addNavObstacle(center, SpaceVect(radius*2.0, radius*2.0));
    }
    else{
        body = cpBodyNew(mass, circleMomentOfInertia(mass, radius));
		cpSpaceAddBody(space, body);
	}

	cpBodySetPos(body, center);

    shape = cpCircleShapeNew(body, radius, cpvzero);
	cpSpaceAddShape(space, shape);
    
    setShapeProperties(shape, layers, type, sensor);
    
	shape->data = obj;
	body->data = obj;

    return make_pair(shape,body);
}

pair<cpShape*, cpBody*> GSpace::createRectangleBody(
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

	cpBody* body;
	cpShape* shape;

	if (mass <= 0.0) {
		body = cpBodyNewStatic();
		if (type == GType::environment || type == GType::wall)
			addNavObstacle(center, dim);
	}
	else {
		body = cpBodyNew(mass, rectangleMomentOfInertia(mass, dim));
		cpSpaceAddBody(space, body);
	}
    
	cpBodySetPos(body, center);

	shape = cpBoxShapeNew(body, dim.x, dim.y);
	cpSpaceAddShape(space, shape);

	setShapeProperties(shape, layers, type, sensor);

	shape->data = obj;
	body->data = obj;

	return make_pair(shape, body);
}

void GSpace::logHandler(const string& base, cpArbiter* arb)
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
		player->onBulletCollide(_bullet);
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
		_enemy_agent->onBulletCollide(bullet);
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
	bool _sensor = cpShapeGetSensor(wall->bodyShape);

	if (_b && _w && !_sensor) {
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

int GSpace::environmentAreaSensorBegin(GObject* obj, GObject* areaSensor)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectContact(obj);
	}

	return 1;
}

int GSpace::environmentAreaSensorEnd(GObject* areaSensor, GObject* obj)
{
	AreaSensor* _s = dynamic_cast<AreaSensor*>(areaSensor);

	if (_s && obj) {
		_s->onEnvironmentalObjectEndContact(obj);
	}

	return 1;
}

//END PHYSICS

//BEGIN SENSORS

const GType GSpace::interactibleObjects = enum_bitwise_or(GType, npc, environment);
const GType GSpace::obstacles = enum_bitwise_or5(GType, wall, enemy, environment, npc, player);

struct FeelerData
{
	//input data
	const GObject* agent = nullptr;
	unsigned int gtype;

	//output data
	double distance = 1.0;
	GObject* result = nullptr;
};

void feelerCallback(cpShape *shape, cpFloat t, cpVect n, void *data)
{
	FeelerData* queryData = static_cast<FeelerData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && (to_uint(shape->collision_type) & queryData->gtype) && obj != queryData->agent && t < queryData->distance && t != 0.0) {
		queryData->distance = t;
		queryData->result = obj;
	}
}

struct PointQueryData
{
	//input data
	const GObject* agent;
	unsigned int gtype;

	//output
	GObject* result = nullptr;
};

void pointQueryCallback(cpShape *shape, void *data)
{
	PointQueryData* queryData = static_cast<PointQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && (to_uint(shape->collision_type) & queryData->gtype)) {
		queryData->result = obj;
	}
}

struct ShapeQueryData
{
	const GObject* agent;
	unsigned gtype;

	set<GObject*> results;
};

void shapeQueryCallback(cpShape *shape, cpContactPointSet *points, void *data)
{
	ShapeQueryData* queryData = static_cast<ShapeQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && obj != queryData->agent && (to_uint(shape->collision_type) & queryData->gtype)) {
		queryData->results.insert(obj);
	}
}

struct FeelerQueryData
{
	const GObject* agent;
	unsigned int gtype;
	cpBody* queryBody;

	SpaceFloat distance = 0.0;
};

void feelerQueryCallback(cpShape* shape, cpContactPointSet* points, void* data)
{
	FeelerQueryData* queryData = static_cast<FeelerQueryData*>(data);
	GObject* obj = to_gobject(shape->data);

	if (obj && obj != queryData->agent && (to_uint(shape->collision_type) & queryData->gtype)) {
		for_irange(i, 0, points->count) {
			SpaceVect local = cpBodyWorld2Local(queryData->queryBody, points->points[i].point);
			queryData->distance = min(queryData->distance, local.x);
		}
	}
}

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype) const
{
	return distanceFeeler(agent, _feeler, gtype, PhysicsLayers::all);
}

SpaceFloat GSpace::distanceFeeler(const GObject * agent, SpaceVect _feeler, GType gtype, PhysicsLayers layers) const
{
    SpaceVect start = agent->getPos();
    SpaceVect end = start + _feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);
        
    return queryData.distance*_feeler.length();
}

SpaceFloat GSpace::wallDistanceFeeler(const GObject * agent, SpaceVect feeler) const
{
    return distanceFeeler(agent, feeler, GType::wall);
}

SpaceFloat GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect _feeler) const
{
	SpaceFloat d = distanceFeeler(
		agent,
		_feeler,
		obstacles,
		agent->getCrntLayers()
	);

    return min(d, agent->isOnFloor() ? trapFloorDistanceFeeler(agent,_feeler) : _feeler.length());
}

SpaceFloat GSpace::obstacleDistanceFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	SpaceVect start = agent->getPos();
	SpaceVect center = agent->getPos() + feeler*0.5;
	SpaceVect dimensions(feeler.length(), width);

	return rectangleFeelerQuery(
		agent,
		center,
		dimensions,
		obstacles,
		PhysicsLayers::all,
		feeler.toAngle()
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
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

    return queryData.distance < 1.0;
}

GObject* GSpace::objectFeeler(const GObject * agent, SpaceVect feeler, GType gtype, PhysicsLayers layers) const
{
	SpaceVect start = agent->getPos();
	SpaceVect end = start + feeler;
	FeelerData queryData = { agent, to_uint(gtype) };

	cpSpaceSegmentQuery(space, start, end, to_uint(layers), 0, feelerCallback, &queryData);

	return queryData.result;
}

bool GSpace::wallFeeler(const GObject * agent, SpaceVect _feeler) const
{
    return feeler(agent, _feeler, GType::wall);
}

bool GSpace::obstacleFeeler(const GObject * agent, SpaceVect feeler, SpaceFloat width) const
{
	return obstacleDistanceFeeler(agent, feeler, width) < feeler.length();
}

bool GSpace::obstacleFeeler(const GObject * agent, SpaceVect _feeler) const
{
	return feeler(
		agent,
		_feeler,
		obstacles,
		agent->getCrntLayers()
	);
}

InteractibleObject* GSpace::interactibleObjectFeeler(const GObject* agent, SpaceVect feeler) const
{
	GObject* obj = objectFeeler(
		agent,
		feeler,
		interactibleObjects,
		agent->getCrntLayers()
	);

	return dynamic_cast<InteractibleObject*>(obj);
}

bool GSpace::lineOfSight(const GObject* agent, const GObject * target) const
{
    SpaceVect feeler_displacement = target->getPos() - agent->getPos();

	return !feeler(
		agent,
		feeler_displacement,
		enum_bitwise_or(GType,environment, wall),
		PhysicsLayers::eyeLevel
	);
}

GObject * GSpace::queryAdjacentTiles(SpaceVect pos, GType type, PhysicsLayers layers, type_index t)
{
	enum_foreach(Direction, d, right, end)
	{
		GObject* result = pointQuery(pos + dirToVector(d), type, layers);

		if (result && type_index(typeid(*result)) == t) {
			return result;
		}
	}

	return nullptr;
}

GObject * GSpace::pointQuery(SpaceVect pos, GType type, PhysicsLayers layers)
{
	PointQueryData queryData = { nullptr, to_uint(type) };

	cpSpacePointQuery(space, pos, to_uint(layers), 0, pointQueryCallback, &queryData);

	return queryData.result;
}

bool GSpace::rectangleQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle)
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return !data.results.empty();
}

SpaceFloat GSpace::rectangleFeelerQuery(const GObject* agent, SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle) const
{
	cpBody* body = cpBodyNewStatic();
	FeelerQueryData data = { agent, to_uint(type), body, dimensions.x };
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, feelerQueryCallback, &data);
	cpBodyFree(body);

	return data.distance;
}

set<GObject*> GSpace::rectangleObjectQuery(SpaceVect center, SpaceVect dimensions, GType type, PhysicsLayers layers, SpaceFloat angle)
{
	ShapeQueryData data = { nullptr, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* area = cpBoxShapeNew(body, dimensions.x, dimensions.y);

	cpBodySetPos(body, center);
	cpBodySetAngle(body, angle);
	setShapeProperties(area, layers, GType::none, false);

	cpSpaceShapeQuery(space, area, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
}

bool GSpace::obstacleRadiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers)
{
	return radiusQuery(agent, center, radius, type, layers).size() > 0;
}

set<GObject*> GSpace::radiusQuery(const GObject* agent, SpaceVect center, SpaceFloat radius, GType type, PhysicsLayers layers)
{
	ShapeQueryData data = { agent, to_uint(type) };
	cpBody* body = cpBodyNewStatic();
	cpShape* circle = cpCircleShapeNew(body, radius, SpaceVect::zero);

	cpBodySetPos(body, center);
	setShapeProperties(circle, layers, GType::none, false);

	cpSpaceShapeQuery(space, circle, shapeQueryCallback, &data);
	cpBodyFree(body);

	return data.results;
}

//END SENSORS
