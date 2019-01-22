//
//  scenes.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"


#include "App.h"
#include "controls.h"
#include "Dialog.hpp"
#include "functional.hpp"
#include "GAnimation.hpp"
#include "GObject.hpp"
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "PlayScene.hpp"
#include "scenes.h"
#include "types.h"
#include "value_map.hpp"

const int GScene::dialogEdgeMargin = 30;
const bool GScene::scriptLog = false;

string GScene::crntSceneName;
string GScene::crntReplayName;
bool GScene::suppressGameOver = false;

GScene* GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
    
    if(it == adapters.end()){
        log("runScene: %s not found", name.c_str());
		return nullptr;
    }
    else
    {
		crntSceneName = name;
		return it->second();
    }
}

void GScene::runSceneWithReplay(const string& sceneName, const string& replayName)
{
	GScene* _scene = runScene(sceneName);

	PlayScene* ps = dynamic_cast<PlayScene*>(_scene);

	if (ps) {
		ps->loadReplayData(replayName);
		crntReplayName = replayName;
	}
	else {
		log("GScene::runSceneWithReplay: not a PlayScene!");
	}
}

void GScene::restartScene()
{
	runScene(crntSceneName);
}

void GScene::restartReplayScene()
{
	runSceneWithReplay(crntSceneName,crntReplayName);
}

vector<GScene::MapEntry> GScene::singleMapEntry(const string& mapName)
{
	return {
		{mapName, IntVec2(0,0)}
	};
}

const IntVec2 GScene::getRoomOffset(IntVec2 roomSize, int roomGridX, int roomGridY)
{
	return IntVec2(roomGridX * roomSize.first, roomGridY * roomSize.second);
}

GScene::GScene(const string& sceneName, const vector<MapEntry>& maps) :
maps(maps),
sceneName(sceneName),
ctx(make_unique<Lua::Inst>("scene")),
control_listener(make_unique<ControlListener>())
{
	
	multiInit.insertWithOrder(
		wrap_method(GScene, installLuaShell, this),
		to_int(initOrder::core)
	);
	//Updater has to be scheduled at init time.
    multiInit.insertWithOrder(
        wrap_method(Node,scheduleUpdate,this),
        to_int(initOrder::core)
    );
	multiInit.insertWithOrder(
		wrap_method(GScene, loadMaps, this),
		to_int(initOrder::mapLoad)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, processAdditions, this),
		to_int(initOrder::loadObjects)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, runScriptInit, this),
		to_int(initOrder::postLoadObjects)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, initEnemyStats, this),
		to_int(initOrder::postLoadObjects)
	);

	multiUpdate.insertWithOrder(
		wrap_method(GScene, queueActions, this),
		to_int(updateOrder::queueActions)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, checkPendingScript, this),
		to_int(updateOrder::runShellScript)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, runScriptUpdate, this),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::spriteUpdate),
		to_int(updateOrder::spriteUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::lightmapUpdate),
		to_int(updateOrder::lightmapUpdate)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, renderSpace, this),
		to_int(updateOrder::renderSpace)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::sceneUpdate),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::hudUpdate),
		to_int(updateOrder::hudUpdate)
	);

	enum_foreach(updateOrder, order, begin, end)
	{
		actions.insert_or_assign(order, vector<function<void()>>());
	}

    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for_irange(i,sceneLayers::begin,sceneLayers::end){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }

	gspace = new GSpace(this);

	if (!sceneName.empty())
	{
		string scriptPath = "scripts/scenes/" + sceneName + ".lua";

		if (!FileUtils::getInstance()->isFileExist(scriptPath) && scriptLog) {
			log("GScene: %s script does not exist.", sceneName.c_str());
		}
		else {
			ctx->runFile(scriptPath);
		}
	}

	control_listener->addPressListener(ControlAction::displayMode, bind(&GScene::cycleDisplayMode, this));
}

GScene::~GScene()
{
	isExit = true;
	spaceUpdateThread->join();

	delete gspace;
}

bool GScene::init()
{
    Scene::init();

	lightmapRender = initRenderTexture(sceneLayers::lightmap, BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	colorFilterRender = initRenderTexture(sceneLayers::screenspaceColorFilter, BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });
	spaceRender = initRenderTexture(sceneLayers::space);

	lightmapDrawNode = DrawNode::create();
	lightmapDrawNode->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	getLayer(sceneLayers::lightmap)->addChild(lightmapDrawNode);

	lightmapBackground = DrawNode::create();
	getLayer(sceneLayers::lightmapBackground)->addChild(lightmapBackground);
	lightmapBackground->setVisible(false);
	lightmapBackground->drawSolidRect(Vec2::ZERO, Vec2(App::width, App::height), Color4F::WHITE);

	colorFilterDraw = DrawNode::create();
	setColorFilter(Color4F::WHITE);
	getLayer(sceneLayers::screenspaceColorFilter)->addChild(colorFilterDraw);

	//Apply zoom to adjust viewable area size.
	float baseViewWidth = App::width * App::tilesPerPixel;
	spaceZoom = baseViewWidth / App::viewWidth;

	getLayer(sceneLayers::space)->setScale(spaceZoom);
	getLayer(sceneLayers::lightmap)->setScale(spaceZoom);

    for_irange(i,to_int(sceneLayers::begin),sceneLayers::end){
		if (i != to_int(sceneLayers::space) && i != to_int(sceneLayers::lightmap) && i != to_int(sceneLayers::screenspaceColorFilter)) {
			addChild(layers.at(i), i);
		}
		else {
			layers.at(i)->setActionManager(Director::getInstance()->getActionManager());
			layers.at(i)->setEventDispatcher(Director::getInstance()->getEventDispatcher());
			layers.at(i)->setScheduler(Director::getInstance()->getScheduler());
		}
	}
    
    multiInit();
    
	spaceUpdatesToRun.store(0);
	spaceUpdateThread = make_unique<thread>(&GScene::spaceUpdateMain, this);

    return true;
}

void GScene::update(float dt)
{	
	if (!isPaused) {
		spaceUpdatesToRun.fetch_add(1);
		multiUpdate();
	}
}

GScene* GScene::getReplacementScene()
{
	return Node::ccCreate<GScene>(sceneName, maps);
}

GSpace* GScene::getSpace()
{
	return gspace;
}

void GScene::setPaused(bool p){
    isPaused = p;
}

void GScene::createDialog(const string& res, bool autoAdvance)
{
	dialog = Node::ccCreate<Dialog>();
	dialog->setDialog(res);
	dialog->setPosition(dialogPosition());
	getLayer(sceneLayers::dialog)->addChild(dialog);

	//This options are not actually mutually exclusive, but for simplicity just use a flag
	//to choose one.
	dialog->setAutoAdvance(autoAdvance);
	dialog->setManualAdvance(!autoAdvance);

	dialog->setEndHandler([this]() -> void {
		stopDialog();
	});
}

void GScene::stopDialog()
{
	if (dialog) {
		getLayer(sceneLayers::dialog)->removeChild(dialog);
		dialog = nullptr;
	}
}

Vec2 GScene::dialogPosition()
{
    return Vec2(App::width/2, Dialog::height/2 + dialogEdgeMargin);
}

bool GScene::isDialogActive()
{
	return dialog != nullptr;
}

void GScene::processAdditions()
{
	gspace->processAdditions();
}

void GScene::addActions(const vector<pair<function<void(void)>, updateOrder>>& _actions)
{
	actionsMutex.lock();

	for (auto entry : _actions) {
		actionsToAdd.push_back(entry);
	}

	actionsMutex.unlock();
}

SpriteID GScene::getSpriteID()
{
	return nextSpriteID.fetch_add(1);
}

LightID GScene::getLightID()
{
	return nextLightID.fetch_add(1);
}

void GScene::addLightSource(LightID id, CircleLightArea light)
{
	RadialGradient* g = Node::ccCreate<RadialGradient>(
		toColor4F(light.color) * light.intensity,
		Color4F(0.0f, 0.0f, 0.0f, 1.0f),
		light.radius * App::pixelsPerTile,
		Vec2::ZERO,
		light.flood
	);

	g->setPosition(toCocos(light.origin) * App::pixelsPerTile);
	g->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	g->setContentSize(CCSize(light.radius * 2.0f * App::pixelsPerTile, light.radius * 2.0f * App::pixelsPerTile));
	getLayer(sceneLayers::lightmap)->addChild(g);

	circleLights.insert_or_assign(id,light);
	lightmapRadials.insert_or_assign(id, g);
}

void GScene::addLightSource(LightID id, AmbientLightArea light)
{
	ambientLights.insert_or_assign(id,light);
}

void GScene::addLightSource(LightID id, ConeLightArea light)
{
	coneLights.insert_or_assign(id, light);
}

void GScene::updateLightSource(LightID id, ConeLightArea light)
{
	coneLights.insert_or_assign(id, light);
}

void GScene::removeLightSource(LightID id)
{
	auto it = lightmapRadials.find(id);
	if (it != lightmapRadials.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it->second);
		lightmapRadials.erase(it);
	}

	circleLights.erase(id);
	ambientLights.erase(id);
}

void GScene::setLightSourcePosition(LightID id, SpaceVect pos)
{
	{
		auto it = circleLights.find(id);
		if (it != circleLights.end()) {
			it->second.origin = pos;
			lightmapRadials.at(it->first)->setPosition(toCocos(pos) * App::pixelsPerTile);
		}
	}
	{
		auto it = ambientLights.find(id);
		if (it != ambientLights.end()) {
			it->second.origin = pos;
		}
	}
}

void GScene::createSprite(SpriteID id, string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	Sprite* s = Sprite::create(path);
	getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
	crntSprites.insert_or_assign(id, s);
}

void GScene::createLoopAnimation(SpriteID id, string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
	anim->loadAnimation(name, frameCount, duration);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	animationSprites.insert_or_assign(id, anim);
}

void GScene::createDrawNode(SpriteID id, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	DrawNode* dn = DrawNode::create();
	drawNodes.insert_or_assign(id, dn);
	getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
}

void GScene::createAgentSprite(SpriteID id, string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
	anim->loadAnimation(path, isAgentAnimation);
	agentSprites.insert_or_assign(id, anim);
	getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
}

void GScene::loadAgentAnimation(SpriteID id, string path, bool isAgentAnimation)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->loadAnimation(path, isAgentAnimation);
	}
}

void GScene::setAgentAnimationDirection(SpriteID id, Direction d)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setDirection(d);
	}
}

void GScene::setAgentAnimationFrame(SpriteID id, int frame)
{
	auto it = agentSprites.find(id);
	if (it != agentSprites.end()) {
		PatchConAnimation* anim = it->second;
		anim->setFrame(frame);
	}
}

void GScene::clearDrawNode(SpriteID id)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->clear();
	}
}

void GScene::drawSolidRect(SpriteID id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidRect(lowerLeft, upperRight, color);
	}
}

void GScene::drawSolidCone(SpriteID id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCone(center, radius, startAngle, endAngle, segments, color);
	}
}

void GScene::drawSolidCircle(SpriteID id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
	auto it = drawNodes.find(id);
	if (it != drawNodes.end()) {
		it->second->drawSolidCircle(center, radius, angle, segments, color);
	}
}

void GScene::runSpriteAction(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(generator());
	}
}

void GScene::stopSpriteAction(SpriteID id, cocos_action_tag action)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopActionByTag(to_int(action));
	}
}

void GScene::stopAllSpriteActions(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->stopAllActions();
	}
}

void GScene::removeSprite(SpriteID id)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->removeFromParent();
		_removeSprite(id);
	}
}

void GScene::removeSpriteWithAnimation(SpriteID id, ActionGeneratorType generator)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->runAction(Sequence::createWithTwoActions(generator(), RemoveSelf::create()));
	}
}

void GScene::setSpriteVisible(SpriteID id, bool val)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setVisible(val);
	}
}

void GScene::setSpriteOpacity(SpriteID id, unsigned char op)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setOpacity(op);
	}
}

void GScene::setSpriteTexture(SpriteID id, string path)
{
	auto it = crntSprites.find(id);
	if (it != crntSprites.end()) {
		Sprite* s = dynamic_cast<Sprite*>(it->second);
		if (s) {
			s->setTexture(path);
		}
	}
}

void GScene::setSpriteAngle(SpriteID id, float cocosAngle)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setRotation(cocosAngle);
	}
}

void GScene::setSpritePosition(SpriteID id, Vec2 pos)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setPosition(pos);
	}
}

void GScene::setSpriteZoom(SpriteID id, float zoom)
{
	Node* node = getSpriteAsNode(id);
	if (node) {
		node->setScale(zoom);
	}
}

Node* GScene::getSpriteAsNode(SpriteID id)
{
	{
		auto it = crntSprites.find(id);
		if (it != crntSprites.end()) {
			return it->second;
		}
	}
	{
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			return it->second;
		}
	}
	{
		auto it = animationSprites.find(id);
		if (it != animationSprites.end()) {
			return it->second;
		}
	}
	{
		auto it = agentSprites.find(id);
		if (it != agentSprites.end()) {
			return it->second;
		}
	}
	return nullptr;
}

void GScene::_removeSprite(SpriteID id)
{
	crntSprites.erase(id);
	drawNodes.erase(id);
	animationSprites.erase(id);
	agentSprites.erase(id);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
	cameraArea = calculateCameraArea(v);
		
	getSpaceLayer()->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom - App::width/2,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom - App::height/2
	);
	getLayer(sceneLayers::lightmap)->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom - App::width/2,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom - App::height/2
	);
}

SpaceVect GScene::getMapSize()
{
	return SpaceVect(dimensions.first, dimensions.second);
}

SpaceRect GScene::getCameraArea()
{
	return cameraArea;
}

const vector<SpaceRect>& GScene::getMapAreas()
{
	return mapAreas;
}

const vector<bool>& GScene::getMapAreasVisited()
{
	return mapAreasVisited;
}

int GScene::getMapLocation(SpaceRect r)
{
	return getAreaIndex(mapAreas, r);
}

bool GScene::isInCameraArea(SpaceRect r)
{
	return cameraArea.intersectsRect(r);
}

bool GScene::isInPlayerRoom(SpaceVect v)
{
	return isInArea(mapAreas, v, crntMap);
}

int GScene::getPlayerRoom()
{
	return crntMap;
}

void GScene::teleportToDoor(string name)
{
	gspace->addObjectAction([=]()->void {
		gspace->teleportPlayerToDoor(name);
	});
}

Layer* GScene::getLayer(sceneLayers layer)
{
	auto it = layers.find(to_int(layer));
	if (it == layers.end()) return nullptr;
	return it->second;
}

void GScene::loadMaps()
{
	for(MapEntry entry : maps)
	{
		loadMap(entry);
	}

	gspace->setSize(dimensions.first, dimensions.second);

	if (dimensions.first > 0 && dimensions.second > 0) {
		gspace->addWallBlock(SpaceVect(-1.0, 0.0), SpaceVect(0.0, dimensions.second));
		gspace->addWallBlock(SpaceVect(dimensions.first, 0.0), SpaceVect(dimensions.first + 1.0, dimensions.second));
		gspace->addWallBlock(SpaceVect(0.0, dimensions.second), SpaceVect(dimensions.first, dimensions.second + 1.0));
		gspace->addWallBlock(SpaceVect(0.0, -1.0), SpaceVect(dimensions.first, 0.0));
	}
}

void GScene::loadMap(const MapEntry& mapEntry)
{
	string mapResPath = "maps/" + mapEntry.first + ".tmx";
	TMXTiledMap* tileMap = nullptr;

	if (FileUtils::getInstance()->isFileExist(mapResPath)) {
		tileMap = TMXTiledMap::create(mapResPath);
	}

	if (tileMap) {
		log("Map %s loaded.", mapResPath.c_str());
	}
	else {
		log("Map %s not found.", mapResPath.c_str());
		return;
	}

	Vec2 llCorner = toCocos(mapEntry.second);
	CCSize mapSize = tileMap->getMapSize();
	SpaceRect mapRect(llCorner.x, llCorner.y, mapSize.width, mapSize.height);

	tilemaps.pushBack(tileMap);
	mapAreas.push_back(mapRect);
	gspace->addMapArea(mapRect);
	mapAreasVisited.push_back(false);

	getSpaceLayer()->positionAndAddNode(
		tileMap,
		to_int(GraphicsLayer::map),
		llCorner * App::pixelsPerTile,
		1.0f
	);

	loadPaths(*tileMap, mapEntry.second);
	loadWaypoints(*tileMap, mapEntry.second);
	loadFloorSegments(*tileMap, mapEntry.second);
	loadMapObjects(*tileMap, mapEntry.second);
	loadWalls(*tileMap, mapEntry.second);

	cocos2d::CCSize size = tileMap->getMapSize();

	dimensions = IntVec2(
		max(dimensions.first, to_int(size.width) + mapEntry.second.first),
		max(dimensions.second, to_int(size.height) + mapEntry.second.second)
	);
}

void GScene::loadMapObjects(const TMXTiledMap& map, IntVec2 offset)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"), offset);
    }
}

void GScene::loadPaths(const TMXTiledMap& map, IntVec2 offset)
{
	Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();

	if (!map.getObjectGroup("paths")) {
		return;
	}

	ValueVector paths = map.getObjectGroup("paths")->getObjects();

	for(const Value& value: paths)
	{
		Path crntPath;
		ValueMap asMap = value.asValueMap();

		string name = asMap.at("name").asString();
		ValueVector points = asMap.at("polylinePoints").asValueVector();
		SpaceVect origin(asMap.at("x").asFloat(), asMap.at("y").asFloat());

		for(auto const& point: points)
		{
			crntPath.push_back(SpaceVect(
				(origin.x + point.asValueMap().at("x").asFloat()) / App::pixelsPerTile + offset.first,
				(origin.y - point.asValueMap().at("y").asFloat()) / App::pixelsPerTile + offset.second
			));
		}
		gspace->addPath(name, crntPath);
	}
}

void GScene::loadWaypoints(const TMXTiledMap& map, IntVec2 offset)
{
	Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();

	if (!map.getObjectGroup("waypoints")) {
		return;
	}

	ValueVector waypoints = map.getObjectGroup("waypoints")->getObjects();

	for (const Value& value : waypoints)
	{
		ValueMap asMap = value.asValueMap();
		SpaceRect rect = getUnitspaceRectangle(asMap, offset);

		string name = asMap.at("name").asString();

		gspace->addWaypoint(name, rect.center);
	}
}

void GScene::loadFloorSegments(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* floor = map.getObjectGroup("floor");
	if (!floor)
		return;

	loadObjectGroup(map.getObjectGroup("floor"), offset);
}

void GScene::loadObjectGroup(TMXObjectGroup* group, IntVec2 offset)
{
	const ValueVector& objects = group->getObjects();

	for(const Value& obj: objects)
	{
		ValueMap objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject(objAsMap);
	}
}

void GScene::loadWalls(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* walls = map.getObjectGroup("walls");
	if (!walls)
		return;

	for(const Value& obj: walls->getObjects())
	{
		const ValueMap& objAsMap = obj.asValueMap();
		SpaceRect area = getUnitspaceRectangle(objAsMap, offset);
		gspace->addWallBlock(area);
	}
}

void GScene::initEnemyStats()
{
	gspace->setInitialObjectCount();
}

void GScene::spaceUpdateMain()
{
	while (!isExit)
	{
		while (!isExit && spaceUpdatesToRun.load() > 0) {
			gspace->update();
			spaceUpdatesToRun.fetch_sub(1);
		}

		this_thread::sleep_for(chrono::duration<int,milli>(1));
	}
}

void GScene::updateMapVisibility(SpaceVect playerPos)
{
	for (int i = 0; i < tilemaps.size() && mapAreas.size(); ++i){
		tilemaps.at(i)->setVisible(
			isInCameraArea(mapAreas.at(i)) &&
			mapAreasVisited.at(i)
		);

		if (mapAreas.at(i).containsPoint(playerPos)) {
			crntMap = i;
			mapAreasVisited.at(i) = true;
		}
	}
}

void GScene::renderSpace()
{
	spaceRender->setVisible(display != displayMode::lightmap);
	lightmapRender->setVisible(display != displayMode::base);
	lightmapBackground->setVisible(display == displayMode::lightmap);

	for (TimedLoopAnimation* anim : animationSprites | boost::adaptors::map_values) {
		anim->update();
	}

	redrawLightmap();
}

void GScene::redrawLightmap()
{
	lightmapDrawNode->clear();

	CCRect cameraPix = getCameraArea().toPixelspace();

	lightmapDrawNode->drawSolidRect(
		cameraPix.getLowerCorner(),
		cameraPix.getUpperCorner(),
		ambientLight
	);

	for (pair<unsigned int,CircleLightArea> lightEntry : circleLights)
	{
		Vec2 originPix = toCocos(lightEntry.second.origin) * App::pixelsPerTile;
		float radiusPix = lightEntry.second.radius * App::pixelsPerTile;

		lightmapRadials.at(lightEntry.first)->setVisible(
			isInPlayerRoom(lightEntry.second.origin) && cameraPix.intersectsCircle(originPix, radiusPix)
		);
	}

	for (AmbientLightArea light : ambientLights | boost::adaptors::map_values)
	{
		Color4F color = toColor4F(light.color) * light.intensity;
		Vec2 halfDim = toCocos(light.dimensions) / 2.0f * App::pixelsPerTile;
		Vec2 center = toCocos(light.origin) * App::pixelsPerTile;

		if (isInPlayerRoom(light.origin) && cameraPix.intersectsRect(CCRect(center.x - halfDim.x, center.y - halfDim.y, halfDim.x * 2.0f, halfDim.y * 2.0f))) {
			lightmapDrawNode->drawSolidRect(center - halfDim, center + halfDim, color);
		}
	}

	for (ConeLightArea light : coneLights | boost::adaptors::map_values)
	{
		Color4F color = toColor4F(light.color) * light.intensity;
		Vec2 center = toCocos(light.origin) * App::pixelsPerTile;
		float diameter = light.radius * 2.0f * App::pixelsPerTile;

		CCRect bounds = SpaceRect(light.origin, SpaceVect(light.radius * 2.0, light.radius * 2.0)).toPixelspace();
		
		if (isInPlayerRoom(light.origin) && cameraPix.intersectsRect(bounds)) {
			lightmapDrawNode->drawSolidCone(center, light.radius*App::pixelsPerTile, light.startAngle, light.endAngle, 128, color);
		}
	}
}

void GScene::cycleDisplayMode()
{
	enum_increment(displayMode, display);
	if (display == displayMode::end) {
		display = displayMode::begin;
	}
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer, BlendFunc blend)
{
	RenderTexture* rt = initRenderTexture(sceneLayer);
	rt->getSprite()->setBlendFunc(blend);
	return rt;
}

RenderTexture* GScene::initRenderTexture(sceneLayers sceneLayer)
{
	RenderTexture* rt = RenderTexture::create(App::width, App::height);
	rt->setPosition(App::width / 2.0f, App::height / 2.0f);
	addChild(rt, to_int(sceneLayer));
	rt->addChild(getLayer(sceneLayer));
	rt->setClearColor(Color4F(0.0f, 0.0f, 0.0f, 0.0f));
	rt->setAutoDraw(true);
	rt->setClearFlags(GL_COLOR_BUFFER_BIT);
	return rt;
}

void GScene::installLuaShell()
{
	luaShell = Node::ccCreate<LuaShell>();
	luaShell->setVisible(false);

	control_listener->addPressListener(
		ControlAction::scriptConsole,
		[=]() -> void {luaShell->toggleVisible(); }
	);
	
	control_listener->addPressListener(
		ControlAction::enter,
		[=]() -> void {if (luaShell->isVisible()) pendingScript = luaShell->getText(); }
	);

	getLayer(GScene::sceneLayers::luaShell)->addChild(luaShell, 1);
}

void GScene::checkPendingScript()
{
	if (!pendingScript.empty()) {
		App::lua->runString(pendingScript);
		pendingScript.clear();
	}
}

void GScene::runScriptInit()
{
    ctx->callIfExistsNoReturn("init");
}
void GScene::runScriptUpdate()
{
    ctx->callIfExistsNoReturn("update");
}

void GScene::queueActions()
{
	actionsMutex.lock();

	for (auto entry : actionsToAdd)
	{
		actions.at(entry.second).push_back(entry.first);
	}
	actionsToAdd.clear();

	actionsMutex.unlock();
}

void GScene::runActionsWithOrder(updateOrder order)
{
	vector<function<void(void)>>& _actions = actions.at(order);

	for (auto it = _actions.begin(); it != _actions.end();++it)
	{
		(*it)();
	}
	_actions.clear();
}

void GScene::setColorFilter(const Color4F& color)
{
	colorFilterDraw->clear();
	colorFilterDraw->drawSolidRect(-1.0f*Vec2(App::width/2, App::height/2), Vec2(App::width/2, App::height/2), color);
}
