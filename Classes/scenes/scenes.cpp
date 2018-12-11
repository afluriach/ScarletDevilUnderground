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
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "PlayScene.hpp"
#include "scenes.h"
#include "types.h"
#include "value_map.hpp"

const int GScene::dialogEdgeMargin = 30;

GScene* GScene::crntScene;
string GScene::crntSceneName;
string GScene::crntReplayName;

void GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
    
    if(it == adapters.end()){
        log("runScene: %s not found", name.c_str());
    }
    else
    {
		crntSceneName = name;
		it->second();
    }
}

void GScene::runSceneWithReplay(const string& sceneName, const string& replayName)
{
	runScene(sceneName);

	PlayScene* ps = dynamic_cast<PlayScene*>(GScene::crntScene);

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

GScene::GScene(const string& mapName) :
GScene(mapName, singleMapEntry(mapName))
{
}

GScene::GScene(const string& sceneName, const vector<MapEntry>& maps) :
maps(maps),
ctx(make_unique<Lua::Inst>("scene")),
control_listener(make_unique<ControlListener>())
{
	crntScene = this;
	
	//Updater has to be scheduled at init time.
    multiInit.insertWithOrder(
        wrap_method(Node,scheduleUpdate,this),
        static_cast<int>(initOrder::core)
    );
	multiInit.insertWithOrder(
		wrap_method(GScene, processAdditions, this),
		static_cast<int>(initOrder::loadObjects)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, loadMaps, this),
		static_cast<int>(initOrder::mapLoad)
	);
	multiInit.insertWithOrder(
		wrap_method(GScene, runScriptInit, this),
		static_cast<int>(initOrder::postLoadObjects)
	);

	multiUpdate.insertWithOrder(
		wrap_method(GScene, updateSpace, this),
		static_cast<int>(updateOrder::spaceUpdate)
	);
	multiUpdate.insertWithOrder(
		wrap_method(GScene, runScriptUpdate, this),
		static_cast<int>(updateOrder::sceneUpdate)
	);
    
    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for_irange(i,1,sceneLayers::end){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }

	gspace = new GSpace(getLayer(sceneLayers::space));

	string scriptPath = "scripts/scenes/" + sceneName + ".lua";

	if (!FileUtils::getInstance()->isFileExist(scriptPath))
		log("GScene: %s script does not exist.", sceneName.c_str());
	else
		ctx->runFile(scriptPath);

}

GScene::~GScene()
{
	delete gspace;
}

bool GScene::init()
{
    Layer::init();
    
    for_irange(i,1,sceneLayers::end){
        Layer* l = layers.at(i);
        Node::addChild(l,i);
    }
    
    //Apply zoom to adjust viewable area size.
    float baseViewWidth = App::width * App::tilesPerPixel;
    spaceZoom = baseViewWidth / App::viewWidth;
    //Only apply zoom to space layer.
    getLayer(sceneLayers::space)->setScale(spaceZoom);
    
    multiInit();
    
    return true;
}

void GScene::update(float dt)
{
    if(!isPaused)
        multiUpdate();
}

void GScene::setPaused(bool p){
    isPaused = p;
}

void GScene::createDialog(const string& res, bool autoAdvance)
{
	app->dialog = Dialog::create();
	app->dialog->setDialog(res);
	app->dialog->setPosition(dialogPosition());
	getLayer(sceneLayers::dialog)->addChild(app->dialog);

	//This options are not actually mutually exclusive, but for simplicity just use a flag
	//to choose one.
	app->dialog->setAutoAdvance(autoAdvance);
	app->dialog->setManualAdvance(!autoAdvance);

	app->dialog->setEndHandler([=]() -> void {
		getLayer(sceneLayers::dialog)->removeChild(app->dialog);
		app->dialog = nullptr;
	});
}

void GScene::stopDialog()
{
	if (app->dialog) {
		getLayer(sceneLayers::dialog)->removeChild(app->dialog);
		app->dialog = nullptr;
	}
}

Vec2 GScene::dialogPosition()
{
    return Vec2(App::width/2, Dialog::height/2 + dialogEdgeMargin);
}

void GScene::updateSpace()
{
	gspace->update();
}

void GScene::processAdditions()
{
	gspace->processAdditions();
}

void GScene::move(const Vec2& w)
{
	Vec2 v = w * spaceZoom;
	Vec2 pos = getLayer(sceneLayers::space)->getPosition();

	getLayer(sceneLayers::space)->setPosition(pos - v);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
	getLayer(sceneLayers::space)->setPosition(
		(-App::pixelsPerTile*v.x + App::width / 2)*spaceZoom,
		(-App::pixelsPerTile*v.y + App::height / 2)*spaceZoom
	);
}

SpaceVect GScene::getMapSize()
{
	return SpaceVect(dimensions.first, dimensions.second);
}

Layer* GScene::getLayer(sceneLayers layer)
{
	auto it = layers.find(static_cast<int>(layer));
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

	gspace->addWallBlock(SpaceVect(-1, 0), SpaceVect(0, dimensions.second));
	gspace->addWallBlock(SpaceVect(dimensions.first, 0), SpaceVect(dimensions.first + 1, dimensions.second));
	gspace->addWallBlock(SpaceVect(0, dimensions.second), SpaceVect(dimensions.first, dimensions.second + 1));
	gspace->addWallBlock(SpaceVect(0, -1), SpaceVect(dimensions.first, 0));
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

	getLayer(sceneLayers::space)->positionAndAddNode(
		tileMap,
		static_cast<int>(GraphicsLayer::map),
		toCocos(mapEntry.second) * App::pixelsPerTile,
		1.0f
	);

	loadPaths(*tileMap, mapEntry.second);
	loadRooms(*tileMap, mapEntry.second);
	loadFloorSegments(*tileMap, mapEntry.second);
	loadMapObjects(*tileMap, mapEntry.second);
	loadWalls(*tileMap, mapEntry.second);

	cocos2d::CCSize size = tileMap->getMapSize();

	dimensions = IntVec2(
		max(dimensions.first, static_cast<int>(size.width) + mapEntry.second.first),
		max(dimensions.second, static_cast<int>(size.height) + mapEntry.second.second)
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

	foreach(Value value, paths)
	{
		Path crntPath;
		ValueMap& asMap = value.asValueMap();

		string name = asMap.at("name").asString();
		ValueVector points = asMap.at("polylinePoints").asValueVector();
		SpaceVect origin(asMap.at("x").asFloat() + offset.first, asMap.at("y").asFloat() + offset.second);

		foreach(Value point, points)
		{
			crntPath.push_back(SpaceVect(
				(origin.x + point.asValueMap().at("x").asFloat()) / App::pixelsPerTile,
				(origin.y - point.asValueMap().at("y").asFloat()) / App::pixelsPerTile
			));
		}
		gspace->addPath(name, crntPath);
	}
}

void GScene::loadRooms(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* rooms = map.getObjectGroup("rooms");
	if (!rooms)
		return;

	foreach(Value obj, rooms->getObjects())
	{
		ValueMap& objAsMap = obj.asValueMap();
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

	foreach(Value obj, objects)
	{
		ValueMap& objAsMap = obj.asValueMap();
		convertToUnitSpace(objAsMap, offset);
		gspace->createObject(objAsMap);
	}
}

void GScene::loadWalls(const TMXTiledMap& map, IntVec2 offset)
{
	TMXObjectGroup* walls = map.getObjectGroup("walls");
	if (!walls)
		return;

	foreach(Value obj, walls->getObjects())
	{
		ValueMap& objAsMap = obj.asValueMap();
		cocos2d::CCRect area = getUnitspaceRectangle(objAsMap, offset);
		gspace->addWallBlock(toChipmunk(area.origin), toChipmunk(area.getUpperCorner()));
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
