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
#include "Player.hpp"
#include "PlayScene.hpp"
#include "scenes.h"
#include "types.h"
#include "value_map.hpp"

const int GScene::dialogEdgeMargin = 30;
const bool GScene::scriptLog = false;

string GScene::crntSceneName;
string GScene::crntReplayName;
bool GScene::suppressGameOver = false;

unsigned int GScene::nextLightID = 1;

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
		wrap_method(GScene, checkPendingScript, this),
		to_int(updateOrder::runShellScript)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, runScriptUpdate, this),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, updateMapVisibility, this),
		to_int(updateOrder::sceneUpdate)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, renderSpace, this),
		to_int(updateOrder::renderSpace)
	);
	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::hudUpdate),
		to_int(updateOrder::hudUpdate)
	);

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

	lightmapRender = RenderTexture::create(App::width, App::height);
	lightmapRender->setPosition(App::width / 2.0f, App::height / 2.0f);
	addChild(lightmapRender, to_int(sceneLayers::lightmap));
	lightmapRender->getSprite()->setBlendFunc(BlendFunc{ GL_DST_COLOR,GL_ONE_MINUS_SRC_ALPHA });

	lightmapDrawNode = DrawNode::create();
	lightmapDrawNode->setBlendFunc(BlendFunc{ GL_ONE,GL_ONE });
	getLayer(sceneLayers::lightmap)->addChild(lightmapDrawNode);

	//Apply zoom to adjust viewable area size.
	float baseViewWidth = App::width * App::tilesPerPixel;
	spaceZoom = baseViewWidth / App::viewWidth;

	spaceRender = RenderTexture::create(App::width, App::height);
	spaceRender->setPosition(App::width / 2.0f, App::height / 2.0f);
	addChild(spaceRender, to_int(sceneLayers::space));

	getLayer(sceneLayers::space)->setScale(spaceZoom);
	getLayer(sceneLayers::space)->setVisible(false);

	getLayer(sceneLayers::lightmap)->setScale(spaceZoom);
	getLayer(sceneLayers::lightmap)->setVisible(false);

    for_irange(i,to_int(sceneLayers::begin),sceneLayers::end){
        Layer* l = layers.at(i);
        addChild(l,i);
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

void GScene::addAction(function<void(void)> f, updateOrder order)
{
	actionsMutex.lock();
	actions.push_back(pair<function<void(void)>, updateOrder>(f, order));
	actionsMutex.unlock();
}

void GScene::addAction(pair<function<void(void)>, updateOrder> entry)
{
	actionsMutex.lock();
	actions.push_back(entry);
	actionsMutex.unlock();
}

unsigned int GScene::addLightSource(CircleLightArea light)
{
	unsigned int id = nextLightID++;

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
	return id;
}

unsigned int GScene::addLightSource(AmbientLightArea light)
{
	unsigned int id = nextLightID++;
	ambientLights.insert_or_assign(id,light);
	return id;
}

void GScene::removeLightSource(unsigned int id)
{
	auto it = lightmapRadials.find(id);
	if (it != lightmapRadials.end()) {
		getLayer(sceneLayers::lightmap)->removeChild(it->second);
		lightmapRadials.erase(it);
	}

	circleLights.erase(id);
	ambientLights.erase(id);
}

void GScene::setLightSourcePosition(unsigned int id, SpaceVect pos)
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

unsigned int GScene::createSprite(string path, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	spriteActionsMutex.lock();
	unsigned int id = nextSpriteID++;

	spriteActions.push_back([this, id,path,sceneLayer,pos,zoom]() -> void {
		Sprite* s = Sprite::create(path);
		getSpaceLayer()->positionAndAddNode(s, to_int(sceneLayer), pos, zoom);
		crntSprites.insert_or_assign(id, s);
	});
	spriteActionsMutex.unlock();
	return id;
}

unsigned int GScene::createLoopAnimation(string name, int frameCount, float duration, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	spriteActionsMutex.lock();
	unsigned int id = nextSpriteID++;

	spriteActions.push_back([this, id, name, frameCount, duration, sceneLayer, pos, zoom]() -> void {
		TimedLoopAnimation* anim = Node::ccCreate<TimedLoopAnimation>();
		anim->loadAnimation(name, frameCount, duration);
		getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
		animationSprites.insert_or_assign(id, anim);
	});
	spriteActionsMutex.unlock();
	return id;
}

unsigned int GScene::createDrawNode(GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	spriteActionsMutex.lock();
	unsigned int id = nextSpriteID++;

	spriteActions.push_back([this, id, sceneLayer, pos, zoom]() -> void {
		DrawNode* dn = DrawNode::create();
		drawNodes.insert_or_assign(id, dn);
		getSpaceLayer()->positionAndAddNode(dn, to_int(sceneLayer), pos, zoom);
	});
	spriteActionsMutex.unlock();
	return id;
}

unsigned int GScene::createAgentSprite(string path, bool isAgentAnimation, GraphicsLayer sceneLayer, Vec2 pos, float zoom)
{
	spriteActionsMutex.lock();
	unsigned int id = nextSpriteID++;

	spriteActions.push_back([this, id, path, isAgentAnimation, sceneLayer, pos, zoom]() -> void {
		PatchConAnimation* anim = Node::ccCreate<PatchConAnimation>();
		anim->loadAnimation(path, isAgentAnimation);
		agentSprites.insert_or_assign(id, anim);
		getSpaceLayer()->positionAndAddNode(anim, to_int(sceneLayer), pos, zoom);
	});
	spriteActionsMutex.unlock();
	return id;
}

void GScene::loadAgentAnimation(unsigned int id, string path, bool isAgentAnimation)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, path, isAgentAnimation]() -> void {
		auto it = agentSprites.find(id);
		if (it != agentSprites.end()) {
			PatchConAnimation* anim = it->second;
			anim->loadAnimation(path, isAgentAnimation);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setAgentAnimationDirection(unsigned int id, Direction d)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, d]() -> void {
		auto it = agentSprites.find(id);
		if (it != agentSprites.end()) {
			PatchConAnimation* anim = it->second;
			anim->setDirection(d);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setAgentAnimationFrame(unsigned int id, int frame)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, frame]() -> void {
		auto it = agentSprites.find(id);
		if (it != agentSprites.end()) {
			PatchConAnimation* anim = it->second;
			anim->setFrame(frame);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::clearDrawNode(unsigned int id)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id]() -> void {
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			it->second->clear();
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::drawSolidRect(unsigned int id, Vec2 lowerLeft, Vec2 upperRight, Color4F color)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, lowerLeft, upperRight, color]() -> void {
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			it->second->drawSolidRect(lowerLeft, upperRight, color);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::drawSolidCone(unsigned int id, const Vec2& center, float radius, float startAngle, float endAngle, unsigned int segments, const Color4F &color)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, center, radius, startAngle, endAngle, segments, color]() -> void {
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			it->second->drawSolidCone(center, radius, startAngle, endAngle, segments, color);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::drawSolidCircle(unsigned int id, const Vec2& center, float radius, float angle, unsigned int segments, const Color4F& color)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, center, radius, angle, segments, color]() -> void {
		auto it = drawNodes.find(id);
		if (it != drawNodes.end()) {
			it->second->drawSolidCircle(center, radius, angle, segments, color);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::runSpriteAction(unsigned int id, ActionGeneratorType generator)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, generator]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->runAction(generator());
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::stopSpriteAction(unsigned int id, cocos_action_tag action)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, action]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->stopActionByTag(to_int(action));
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::stopAllSpriteActions(unsigned int id)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->stopAllActions();
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::removeSprite(unsigned int id)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->removeFromParent();
			_removeSprite(id);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::removeSpriteWithAnimation(unsigned int id, ActionGeneratorType generator)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, generator]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->runAction(Sequence::createWithTwoActions(generator(), RemoveSelf::create()));
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpriteVisible(unsigned int id, bool val)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, val]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->setVisible(val);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpriteOpacity(unsigned int id, unsigned char op)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, op]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->setOpacity(op);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpriteTexture(unsigned int id, string path)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this,id, path]()->void {
		auto it = crntSprites.find(id);
		if (it != crntSprites.end()) {
			Sprite* s = dynamic_cast<Sprite*>(it->second);
			if (s) {
				s->setTexture(path);
			}
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpriteAngle(unsigned int id, float cocosAngle)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, cocosAngle]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->setRotation(cocosAngle);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpritePosition(unsigned int id, Vec2 pos)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, pos]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->setPosition(pos);
		}
	});
	spriteActionsMutex.unlock();
}

void GScene::setSpriteZoom(unsigned int id, float zoom)
{
	spriteActionsMutex.lock();
	spriteActions.push_back([this, id, zoom]() -> void {
		Node* node = getSpriteAsNode(id);
		if (node) {
			node->setScale(zoom);
		}
	});
	spriteActionsMutex.unlock();
}

Node* GScene::getSpriteAsNode(unsigned int id)
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

void GScene::_removeSprite(unsigned int id)
{
	crntSprites.erase(id);
	drawNodes.erase(id);
	animationSprites.erase(id);
	agentSprites.erase(id);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
	float heightRatio = 1.0f * App::height / App::width;
	cameraArea.setRect(
		v.x - App::viewWidth / 2,
		v.y - App::viewWidth*heightRatio / 2,
		App::viewWidth,
		App::viewWidth*heightRatio
	);

	getSpaceLayer()->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom
	);
	getLayer(sceneLayers::lightmap)->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom
	);
}

SpaceVect GScene::getMapSize()
{
	return SpaceVect(dimensions.first, dimensions.second);
}

CCRect GScene::getCameraArea()
{
	return cameraArea;
}

const vector<CCRect>& GScene::getMapAreas()
{
	return mapAreas;
}

const vector<bool>& GScene::getMapAreasVisited()
{
	return mapAreasVisited;
}

int GScene::getMapLocation(CCRect r)
{
	for_irange(i, 0, mapAreas.size())
	{
		CCRect mapArea = mapAreas.at(i);

		if (r.getMinX() >= mapArea.getMinX() && r.getMaxX() <= mapArea.getMaxX() &&
			r.getMinY() >= mapArea.getMinY() && r.getMaxY() <= mapArea.getMaxY()) {
			return i;
		}
	}
	return -1;
}

bool GScene::isInCameraArea(CCRect r)
{
	return cameraArea.intersectsRect(r);
}

bool GScene::isInPlayerRoom(SpaceVect v)
{
	if (crntMap == -1) {
		return true;
	}

	CCRect mapArea = mapAreas.at(crntMap);
	return mapArea.containsPoint(Vec2(v.x, v.y));
}

int GScene::getPlayerRoom()
{
	return crntMap;
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
	CCRect mapRect(llCorner.x, llCorner.y, mapSize.width, mapSize.height);

	tilemaps.pushBack(tileMap);
	mapAreas.push_back(mapRect);
	mapAreasVisited.push_back(false);

	getSpaceLayer()->positionAndAddNode(
		tileMap,
		to_int(GraphicsLayer::map),
		llCorner * App::pixelsPerTile,
		1.0f
	);

	loadPaths(*tileMap, mapEntry.second);
	loadWaypoints(*tileMap, mapEntry.second);
	loadRooms(*tileMap, mapEntry.second);
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
		CCRect rect = getUnitspaceRectangle(asMap, offset);

		string name = asMap.at("name").asString();

		gspace->addWaypoint(name, SpaceVect(rect.getMidX(), rect.getMidY()));
	}
}


void GScene::loadRooms(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* rooms = map.getObjectGroup("rooms");
	if (!rooms)
		return;

	for(const Value& obj: rooms->getObjects())
	{
		ValueMap objAsMap = obj.asValueMap();
		gspace->addRoom(getUnitspaceRectangle(objAsMap,offset));
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
		cocos2d::CCRect area = getUnitspaceRectangle(objAsMap, offset);
		gspace->addWallBlock(toChipmunk(area.origin), toChipmunk(area.getUpperCorner()));
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
		while (!isExit && !isPaused && spaceUpdatesToRun.load() > 0) {
			gspace->update();
			spaceUpdatesToRun.fetch_sub(1);
		}

		this_thread::sleep_for(chrono::duration<int,milli>(1));
	}
}


void GScene::updateMapVisibility()
{
	Player* p = gspace->getObjectAs<Player>("player");
	SpaceVect _pos = p ? p->getPos() : SpaceVect::zero;
	Vec2 pos(_pos.x, _pos.y);

	for (int i = 0; i < tilemaps.size() && mapAreas.size(); ++i){
		tilemaps.at(i)->setVisible(
			isInCameraArea(mapAreas.at(i)) &&
			mapAreasVisited.at(i)
		);

		if (p && mapAreas.at(i).containsPoint(pos)) {
			crntMap = i;
			mapAreasVisited.at(i) = true;
		}
	}
}

void GScene::renderSpace()
{
	spriteActionsMutex.lock();
	for (auto f : spriteActions) {
		f();
	}
	spriteActions.clear();

	for (TimedLoopAnimation* anim : animationSprites | boost::adaptors::map_values) {
		anim->update();
	}
	spriteActionsMutex.unlock();

	redrawLightmap();

	Layer* spaceLayer = getSpaceLayer();
	spaceLayer->setVisible(true);

	spaceRender->beginWithClear(0.0f, 0.0f, 0.0f, 0.0f);
	spaceLayer->visit();
	spaceRender->end();

	spaceLayer->setVisible(false);

	Layer* lightmapLayer = getLayer(sceneLayers::lightmap);
	lightmapLayer->setVisible(true);

	lightmapRender->beginWithClear(0.0f, 0.0f, 0.0, 0.0f);
	lightmapLayer->visit();
	lightmapRender->end();

	lightmapLayer->setVisible(false);
}

void GScene::redrawLightmap()
{
	lightmapDrawNode->clear();

	CCRect cameraPix = getCameraArea() * App::pixelsPerTile;

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

void GScene::runActionsWithOrder(updateOrder order)
{
	actionsMutex.lock();
	for (auto it = actions.begin(); it != actions.end();)
	{
		if (it->second == order)
		{
			it->first();
			it = actions.erase(it);
		}
		else
		{
			++it;
		}
	}
	actionsMutex.unlock();
}
