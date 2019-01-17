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
		wrap_method(GScene, updateSpace, this),
		to_int(updateOrder::spaceUpdate)
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
    
    return true;
}

void GScene::update(float dt)
{
    if(!isPaused)
        multiUpdate();
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

void GScene::updateSpace()
{
	gspace->update();
}

void GScene::processAdditions()
{
	gspace->processAdditions();
}

void GScene::addAction(function<void(void)> f, updateOrder order)
{
	actions.push_back(pair<function<void(void)>, updateOrder>(f, order));
}

void GScene::addAction(pair<function<void(void)>, updateOrder> entry)
{
	actions.push_back(entry);
}

unsigned int GScene::addLightSource(CircleLightArea light)
{
	unsigned int id = nextLightID++;
	circleLights.insert_or_assign(id,light);
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
	circleLights.erase(id);
	ambientLights.erase(id);
}

void GScene::setLightSourcePosition(unsigned int id, SpaceVect pos)
{
	{
		auto it = circleLights.find(id);
		if (it != circleLights.end()) {
			it->second.origin = pos;
		}
	}
	{
		auto it = ambientLights.find(id);
		if (it != ambientLights.end()) {
			it->second.origin = pos;
		}
	}
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

	for (CircleLightArea light : circleLights | boost::adaptors::map_values)
	{
		Color4F color = toColor4F(light.color) * light.intensity;		
		Vec2 originPix = toCocos(light.origin) * App::pixelsPerTile;
		float radiusPix = light.radius * App::pixelsPerTile;

		if (isInPlayerRoom(light.origin) && cameraPix.intersectsCircle(originPix, radiusPix)) {
			lightmapDrawNode->drawSolidCircle(originPix, radiusPix, 0.0f, 128, color);
		}
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
}
