//
//  GSpace.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "AreaSensor.hpp"
#include "audio_context.hpp"
#include "Bullet.hpp"
#include "Door.hpp"
#include "Enemy.hpp"
#include "FloorSegment.hpp"
#include "Graph.hpp"
#include "graphics_context.hpp"
//#include "GSpace.hpp"
#include "HUD.hpp"
#include "LuaAPI.hpp"
#include "physics_context.hpp"
#include "PhysicsImpl.hpp"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "RadarSensor.hpp"
#include "SpellDescriptor.hpp"
#include "value_map.hpp"
#include "Wall.hpp"

class RadarObject;

const vector<string> GSpace::scriptFiles = {
	"ai-fsm",
	"ai-functions",
	"ai-packages",
	"bullets",
    "enemy",
    "floors",
	"magic-effects",
	"items",
	"npc",
	"objects",
	"player",
	"spells/fairy-spells",
	"spells/marisa-spells",
	"spells/player-spells",
	"spells/patchouli-spells",
	"spells/reimu-spells",
	"spells/sakuya-spells",
};

unique_ptr<Lua::Inst> GSpace::scriptVM;

local_shared_ptr<agent_properties> GSpace::playerCharacter;

GSpace* GSpace::crntSpace = nullptr;

void GSpace::loadScriptVM()
{
	scriptVM = make_unique<Lua::Inst>("GSpace");
 
    for(string file : scriptFiles){
        scriptVM->runFile("scripts/" + file + ".lua");
    }
}

GSpace* GSpace::getCrntSpace()
{
    return crntSpace;
}

GSpace::GSpace(GScene* gscene) :
	gscene(gscene),
#if use_sound
	audioContext(App::audioContext.get()),
#endif
	graphicsContext(gscene->graphicsContext.get()),
	randomFloat(0.0, 1.0)
{
	world = new b2World(b2Vec2_zero);

	physicsContext = make_unique<physics_context>(this);
	physicsImpl = make_unique<PhysicsImpl>(this);
    physicsImpl->addCollisionHandlers();

	for (type_index t : GObject::trackedTypes) {
		objByType[t] = unordered_set<GObject*>();
	}

    crntSpace = this;
}

GSpace::~GSpace()
{
	isUnloading = true;
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

	delete world;
}

AreaStats& GSpace::getAreaStats()
{
	App::crntState->checkInitAreaState(crntChamber);
	return App::crntState->areaStats.at(crntChamber);
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

SpaceFloat GSpace::getTime() const {
	return elapsedTime;
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

#if USE_TIMERS
	chrono::steady_clock::time_point t1 = chrono::steady_clock::now();
#endif

    //Run inits for recently added objects
    initObjects();
    
#if USE_TIMERS
	chrono::steady_clock::time_point t2 = chrono::steady_clock::now();
#endif

    //physics step
	isInPhysicsStep = true;
	world->Step(
		app::params.secondsPerFrame,
		PhysicsImpl::velocitySteps,
		PhysicsImpl::positionSteps
	);
	isInPhysicsStep = false;
    
	for (auto entry : updateMessages)
		entry();
	updateMessages.clear();

	updateSoundSources();
	updateCamera();

#if USE_TIMERS
	chrono::steady_clock::time_point t3 = chrono::steady_clock::now();
#endif

	vector<sprite_update> spriteUpdates;
	for (GObject* obj : objByUUID | boost::adaptors::map_values) {
		if (obj->isGraphicsObject()) {
			auto result = obj->updateSprite();
			if (result.valid)
				spriteUpdates.push_back(result);
		}
	}
	addGraphicsAction(&graphics_context::spriteSpatialUpdate, spriteUpdates);

	updateSensors();

    for(GObject* obj : updateObjects){
        obj->update();
        obj->runMethodIfAvailable("update");
    }
    
    processRemovals();
    
    //process additions
    processAdditions();
    
	gscene->addActions(sceneActions);
	sceneActions.clear();

	if (frame % app::params.framesPerSecond == 0) {
		App::crntState->incrementElapsedTime();
		++getAreaStats().totalTime;
	}

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
	elapsedTime += app::params.secondsPerFrame;
}

void GSpace::updateSensors()
{
	for (auto radar : radarSensors) {
		radar->update();
	}
}

void GSpace::updateSoundSources()
{
#if use_sound
	auto it = activeSounds.begin();
	while (it != activeSounds.end())
	{
		GObject* obj = it->first;
		ALuint sourceID = it->second;
		if (!audioContext->setSoundSourcePos(sourceID, obj->getPos(), obj->getVel(), obj->getAngle()))
			it = activeSounds.erase(it);
		else
			++it;
	}
#endif
}

//BEGIN OBJECT MANIPULATION


void GSpace::addDynamicLoadObject(const ValueMap& obj)
{
	string name = obj.at("name").asString();
	string type = obj.at("type").asString();
 
    object_params params(obj);
    auto props = app::getObjectProps(type);

	if (name.empty()) {
		log0("Un-named dynamic load object");
	}
	else if (type.empty()) {
		log0("Un-typed dynamic load object");
	}
	else if (!props) {
		log1("Dynamic load object with unknown type %s.", type.c_str());
	}
	else if (dynamicLoadObjects.find(name) != dynamicLoadObjects.end()) {
		log1("Dynamic load object name %s already used.", name.c_str());
	}
	else {
		dynamicLoadObjects.insert_or_assign(name, pair(params, props));
	}
}

gobject_ref GSpace::createDynamicObject(const string& name)
{
	auto it = dynamicLoadObjects.find(name);
	if (it != dynamicLoadObjects.end()) {
        return createObject(it->second.first, it->second.second);
	}
	else {
		log1("Unknown dynamic load object name %s.", name.c_str());
		return nullptr;
	}
}

const object_generator* GSpace::getDynamicObject(const string& name) const
{
	auto it = dynamicLoadObjects.find(name);
	if (it != dynamicLoadObjects.end())
		return &it->second;
	else
		return nullptr;
}

gobject_ref GSpace::createObject(const ValueMap& obj)
{
	string type = getStringOrDefault(obj, "type", "");
    auto props = app::getObjectProps(type);
    object_params params(obj);
    
    if(!props){
        log1("Unknown object type %s!", type);
        return gobject_ref();
    }

    return createObject(params, props);
}

Bullet* GSpace::createBullet(
	const object_params& params,
	const bullet_attributes& attributes,
	local_shared_ptr<bullet_properties> props
) {
	return createObject<Bullet>(params, attributes, props);
}

GObject* GSpace::createObject(const object_params& params, local_shared_ptr<object_properties> props)
{
    if(!GObject::conditionalLoad(this, props, params))
        return nullptr;

    object_properties* objProps = props.get();
    auto it = GObject::propsAdapters.find(typeid(*objProps));

    if(it != GObject::propsAdapters.end()){
        GObject* obj = it->second(this, nextObjUUID++, params, props);
        addObject(obj);
        return obj;
    }
    else{
        log1("Unknown properties type %s.", typeid(*objProps).name());
        return nullptr;
    }
}

void GSpace::addSpatialSound(GObject* sourceObj, ALuint soundSource)
{
	activeSounds.push_back(make_pair(sourceObj, soundSource));
}

void GSpace::removeSpatialSound(ALuint soundSource)
{
	auto it = activeSounds.begin();
	while (it != activeSounds.end()){
		if (it->second == soundSource) {
			activeSounds.erase(it);
			return;
		}
		else {
			++it;
		}
	}
}

void GSpace::removeSpatialSounds(GObject* sourceObj)
{
	auto it = activeSounds.begin();
	while (it != activeSounds.end()) {
		if (it->first == sourceObj) {
			it = activeSounds.erase(it);
		}
		else {
			++it;
		}
	}
}

void GSpace::addWallBlock(const SpaceVect& ll, const SpaceVect& ur)
{
	addWallBlock(SpaceRect(ll.x, ll.y, ur.x - ll.x, ur.y - ll.y));
}

void GSpace::addWallBlock(const SpaceRect& area)
{
	createObject<Wall>(area, app::getWall("Wall"));
}

gobject_ref GSpace::getObjectRef(const string& name) const
{
	auto it = objByName.find(name);
	if (it != objByName.end()) {
		return gobject_ref(it->second);
	}
	else {
		return gobject_ref();
	}
}

gobject_ref GSpace::getObjectRef(unsigned int uuid) const
{
	return getObject(uuid);
}

GObject* GSpace::getObject(const string& name) const
{
	auto it = objByName.find(name);
	if (it != objByName.end()) {
		return it->second;
	}
	else {
		log1("\"%s\" not found!", name);
		return nullptr;
	}
}

GObject* GSpace::getObject(unsigned int uuid) const
{
	return getOrDefault(objByUUID, uuid, to_gobject(nullptr));
}

RoomSensor* GSpace::getRoomSensor(int id) const
{
	return getOrDefault<int,RoomSensor*>(roomSensors, id, nullptr);
}

const unordered_set<GObject*>* GSpace::getObjectsByType(type_index t) const
{
	if(!isTrackedType(t)){
		log1("%s is not a tracked type.", t.name());
		return nullptr;
	}

	return &(objByType.at(t));
}

bool GSpace::isTrackedType(type_index t) const
{
	return GObject::trackedTypes.find(t) != GObject::trackedTypes.end();
}

bool GSpace::isValid(unsigned int uuid) const
{
    return getObject(uuid) != nullptr;
}

bool GSpace::isFutureObject(ObjectIDType uuid) const
{
	return uuid != 0 && uuid > lastAddedUUID && uuid < nextObjUUID;
}

void GSpace::processAdditions()
{
    for(GObject* obj : toAdd)
    {
        if(objByUUID.find(obj->uuid) != objByUUID.end()){
            log1("Object UUID is not unique: %s", obj->toString());
            allocator_delete(obj);
            continue;
        }
        
		obj->initializeBody();

		if (obj->body) {
			obj->setAngle(obj->prevAngle);
			obj->setVel(obj->startingVel);
			obj->setAngularVel(obj->startingAngularVel);
		}

		if (!obj->body) {
			log1("Object %s failed to load physics body!", obj->getName());
			allocator_delete(obj);
			continue;
		}

        obj->initializeGraphics();
		obj->runMethodIfAvailable("initializeGraphics");

		if (isTrackedType(typeid(*obj))) {
			objByType[typeid(*obj)].insert(obj);
		}
  
        if (obj->getMass() < 0.0 && bitwise_and_bool(obj->getCrntLayers(), PhysicsLayers::ground)){
            addNavObstacle(obj->getPos(), obj->getDimensions());
        }

        objByUUID[obj->uuid] = obj;
        objByName[obj->name] = obj;

		if (!isNoUpdateObject(obj) || obj->hasMethod("update")) {
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
        log1("%s not found", name.c_str());
        return;
    }
    
    toRemove.push_back( it->second);
}

void GSpace::removeObject(GObject* obj)
{
	if (!obj) {
		log0("nullptr");
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
	vector<gobject_ref> bullets = getObjectsByTypeAs<Bullet>();

	for (auto ref : bullets)
	{
		ref.getAs<Bullet>()->setBodyVisible(b);
	}
}

void GSpace::addObject(GObject* obj)
{
    if(obj){
        toAdd.push_back(obj);
    }
}

void GSpace::processRemoval(GObject* obj, bool _removeSprite)
{
    obj->removeEffects();
	obj->onRemove();

	if (!isUnloading && obj->name.size() > 0 && crntChamber.size() > 0) {
		getAreaStats().addObjectRemoval(obj->name);
	}

    objByName.erase(obj->name);
    objByUUID.erase(obj->uuid);
	updateObjects.erase(obj);
	removeSpatialSounds(obj);

	if (isTrackedType(typeid(*obj))) {
		objByType[typeid(*obj)].erase(obj);
	}
    
	if (obj->getMass() < 0.0 && bitwise_and_bool(obj->getCrntLayers(), PhysicsLayers::ground)){
		removeNavObstacle(obj->getPos(), obj->getDimensions());
	}

	obj->removePhysicsObjects();
	obj->removeGraphics(_removeSprite);

	allocator_delete(obj);
}

bool GSpace::isNoUpdateObject(GObject* obj)
{
	return
		typeid(*obj) == typeid(Wall) ||
		typeid(*obj) == typeid(FloorSegment) ||
		typeid(*obj) == typeid(Pitfall)
	;
}

void GSpace::initObjects()
{
	for (auto f : initMessages) f();
	initMessages.clear();

    for(GObject* obj: addedLastFrame)
    {
        obj->init();
        obj->scriptInitialize();
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

		unsigned int spriteID = entry.first->getSpriteID();

		processRemoval(entry.first, false);

		addGraphicsAction(&graphics_context::removeSpriteWithAnimation, spriteID, entry.second);
	}
}

void GSpace::setRandomSeed(unsigned int seed)
{
	log1("Random seed %u loaded.", seed);

	randomEngine.seed(seed);
	randomFloat.reset();
}

float GSpace::getRandomFloat() {
	return randomFloat(randomEngine);
}

float GSpace::getRandomFloat(float min, float max) {
	return (min + getRandomFloat() * (max - min));
}

int GSpace::getRandomInt(int min, int max) {
	uniform_int_distribution<int> randomInt(min,max);
	return randomInt(randomEngine);
}

bool GSpace::getRandomBool(){
    return getRandomBool(0.5f);
}

bool GSpace::getRandomBool(float probability){
    return getRandomFloat() < probability;
}

vector<int> GSpace::getRandomShuffle(int n) {
	vector<int> result;

	for_irange(i, 0, n) result.push_back(i);
	shuffle(result.begin(), result.end(), randomEngine);

	return result;
}

ControlInfo GSpace::getControlInfo() const {
	return controlInfo;
}

void GSpace::setControlInfo(ControlInfo info) {
	controlInfo = info;
}

gobject_ref GSpace::createAreaSensor(
	SpaceRect rect,
	GType targets,
	unary_gobject_function onContact,
	unary_gobject_function onEndContact
){
	return createObject<AreaSensorImpl>(rect, targets, onContact, onEndContact);
}

void GSpace::addInitAction(zero_arity_function f)
{
	initMessages.push_back(f);
}

void GSpace::addUpdateAction(zero_arity_function f)
{
	updateMessages.push_back(f);
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

void GSpace::enterWorldSelect()
{
	addSceneAction(
		[this]()->void { this->getSceneAs<PlayScene>()->enterWorldSelect(); }
	);
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

void GSpace::loadScene(string mapName, string start)
{
    addSceneAction(
        [mapName, start]()->void { App::runPlayScene(mapName, start); }
    );
}

void GSpace::teleportPlayerToDoor(string doorName)
{
	Door* d = getObjectAs<Door>(doorName);
	Player* p = getPlayer();

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

void GSpace::updatePlayerMapLocation(int roomID)
{
	crntMap = roomID;
	getAreaStats().roomsVisited.set(roomID);

	addSceneAction(bind(&GScene::updateMapVisibility, gscene, roomID));
}

void GSpace::addMapArea(const SpaceRect& area)
{
	mapAreas.push_back(area);
}

void GSpace::registerRoomMapped(int roomID)
{
	if (roomID >= 0 && roomID < maxRoomsPerChamber) {
		getAreaStats().roomsMapped.set(roomID, true);
		addSceneAction(bind(&GScene::setRoomDiscovered, gscene, roomID));
	}
}

SpaceRect GSpace::getCameraArea()
{
	return cameraArea;
}

void GSpace::updateCamera()
{
	if (auto p = getPlayer()) {
		SpaceVect pos = p->getPos();
		setCameraPosition(pos);
	}
}

void GSpace::setCameraPosition(SpaceVect pos)
{
	cameraArea = calculateCameraArea(pos);
	addSceneAction(bind(&GScene::setUnitPosition, gscene, pos));
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

bool GSpace::isInPlayerRoom(int roomID)
{
	return roomID == crntMap;
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

void GSpace::addRoomSensor(RoomSensor* rs)
{
	roomSensors.insert_or_assign(rs->mapID, rs);
}

void GSpace::removeRoomSensor(RoomSensor* rs)
{
	roomSensors.erase(rs->mapID);
}

Player* GSpace::getPlayer()
{
	auto objects = getObjectsByType(typeid(Player));

#if DEV_MODE
	if (objByUUID.size() > 0)
	{
		if (objects->size() > 1) {
			log0("multiple player objects");
		}
		else if (objects->empty()) {
			log0("no player object!");
		}
	}
#endif

	if (!objects->empty()) {
		return dynamic_cast<Player*>(*objects->begin());
	}
	else {
		return nullptr;
	}
}

gobject_ref GSpace::getPlayerAsRef()
{
	return getPlayer();
}

//END OBJECT MANIPULATION

//BEGIN NAVIGATION
void GSpace::addPath(string name, Path p)
{
	if (paths.find(name) != paths.end()) {
		log1("Duplicate path name %s!", name.c_str());
	}
	paths[name] = make_shared<Path>(p);
}

shared_ptr<const Path> GSpace::getPath(string name) const
{
	auto it = paths.find(name);
	return it != paths.end() ? it->second : nullptr;
}

void GSpace::addWaypoint(string name, const vector<string>& tags, SpaceVect w)
{
	addWaypoint(name, w);

	for (auto tag : tags) {
		addWaypoint(tag, w);
	}
}

void GSpace::addWaypoint(string tag, SpaceVect w)
{
	if (tag.size() > 0) {
		emplaceIfEmpty(waypoints, tag);
		auto it = waypoints.find(tag);
		it->second.push_back(w);
	}
}

SpaceVect GSpace::getWaypoint(string name) const
{
	auto it = waypoints.find(name);

	if (it == waypoints.end() || it->second.empty()) {
		log1("Unknown waypoint %s.", name.c_str());
		return SpaceVect(0, 0);
	}

	if (it->second.size() > 1) {
		log1("more than one waypoint for %s.", name.c_str());
	}

	return it->second.back();
}

const list<SpaceVect>* GSpace::getWaypoints(string name) const
{
	auto it = waypoints.find(name);

	if (it == waypoints.end()) {
		log1("Unknown waypoint %s.", name.c_str());
		return nullptr;
	}
	else {
		return &it->second;
	}
}

SpaceVect GSpace::getRandomWaypoint(string name)
{
	const list<SpaceVect>* w = getWaypoints(name);

	if (!w || w->size() == 0) {
		return SpaceVect::zero;
	}
	else if (w->size() == 1) {
		return w->front();
	}
	else {
		int idx = getRandomInt(0, w->size() - 1);
		return listAt<SpaceVect>(*w, idx);
	}
}

void GSpace::addArea(string name, SpaceRect a)
{
	if (areas.find(name) != areas.end()) {
		log1("Duplicate area name %s!", name);
	}

	areas.insert_or_assign(name, a);
}

SpaceRect GSpace::getArea(string name) const
{
	auto it = areas.find(name);

	if (it != areas.end()) {
		return it->second;
	}
	else {
		log1("Unknown area name %s!", name);
		return SpaceRect();
	}
}

FloorSegment* GSpace::floorPointQuery(SpaceVect pos)
{
	return dynamic_cast<FloorSegment*>(physicsContext->pointQuery(
		pos,
		GType::floorSegment,
		PhysicsLayers::floor
	));
}

Pitfall* GSpace::pitfallPointQuery(SpaceVect pos)
{
	return dynamic_cast<Pitfall*>(physicsContext->pointQuery(
		pos,
		GType::floorSegment,
		PhysicsLayers::belowFloor
	));
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

shared_ptr<const Path> GSpace::pathToTile(IntVec2 begin, IntVec2 end)
{
	vector<SpaceVect> result;
 
    if(isObstacleTile(begin.first, begin.second)){
        log2("Begin point (%d,%d) is an obstacle tile!", begin.first, begin.second);
        return nullptr;
    }
    if(isObstacleTile(end.first, end.second)){
        log2("End point (%d,%d) is an obstacle tile!", end.first, end.second);
        return nullptr;
    }
    if(begin.first == end.first && begin.second == end.second){
        log0("Start and end point are the same!");
        return nullptr;
    }

	vector<pair<int, int>> tileCoords = graph::gridAStar(
		*navMask,
		begin,
		end,
		getSize()
	);

	if (tileCoords.size() < 1) {
		return nullptr;
	}

    result.push_back(toSpaceVectWithCentering(begin));
	result.push_back(toSpaceVectWithCentering(tileCoords[0]));
	SpaceVect direction = toSpaceVectWithCentering(tileCoords[0]) - toSpaceVectWithCentering(begin);

	for_irange(i, 1, tileCoords.size())
	{
		SpaceVect crntTile = toSpaceVectWithCentering(tileCoords[i]);
		SpaceVect crntDirection = crntTile - result.back();

		if (SpaceVect::fuzzyMatch(direction, crntDirection)){
			result.back() = crntTile;
		}
		else {
			result.push_back(crntTile);
			direction = crntDirection;
		}
	}

	SpaceVect endPoint = toSpaceVectWithCentering(tileCoords.back());

	//Endpoint will not be added loop unless it happens to be a turn (i.e. not colinear with the previous tile coord)
	if (!SpaceVect::fuzzyMatch(result.back(), endPoint)) {
		result.push_back(endPoint);
	}

	return make_shared<Path>(result);
}

//END NAVIGATION

LightID GSpace::addLightSource(shared_ptr<LightArea> light, SpaceVect pos, SpaceFloat angle)
{
	LightID id = graphicsContext->getLightID();

	sceneActions.push_back(bind(
		&graphics_context::addPolyLightSource,
		graphicsContext,
		id,
		light,
		pos,
		angle
	));

	return id;
}

void GSpace::removeLightSource(LightID _id)
{
	sceneActions.push_back(bind(
		&graphics_context::removeLightSource,
        graphicsContext,
		_id
	));
}

void GSpace::runSpriteAction(SpriteID id, GraphicsAction action)
{
	addGraphicsAction(
		&graphics_context::runSpriteAction,
		id,
		action.generator
	);
}
