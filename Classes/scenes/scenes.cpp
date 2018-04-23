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
#include "GSpace.hpp"
#include "LuaAPI.hpp"
#include "macros.h"
#include "PlayScene.hpp"
#include "scenes.h"
#include "types.h"

GScene* GScene::crntScene;
string GScene::crntSceneName;

void GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
    
    if(it == adapters.end()){
        log("runScene: %s not found", name.c_str());
    }
    else
    {
        it->second();
		crntSceneName = name;
    }
}

void GScene::restartScene()
{
	runScene(crntSceneName);
}

GScene::GScene() :
control_listener(make_unique<ControlListener>())
{
    //Updater has to be scheduled at init time.
    multiInit.insertWithOrder(
        wrap_method(GScene,initUpdate,this),
        static_cast<int>(initOrder::core)
    );

    crntScene = this;
    
    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for_irange(i,1,sceneLayers::end){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }
}

GScene::~GScene() {}

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

Vec2 GScene::dialogPosition()
{
    return Vec2(App::width/2, Dialog::height/2 + dialogEdgeMargin);
}
    
 bool GScene::isDialogActive(){
    return crntScene->dialogNode != nullptr;
}

GSpaceScene::GSpaceScene()
{
    gspace = new GSpace(getLayer(sceneLayers::space));

    multiInit.insertWithOrder(
        wrap_method(GSpaceScene,processAdditions,this),
        static_cast<int>(initOrder::loadObjects)
    );
    multiUpdate.insertWithOrder(
        wrap_method(GSpaceScene,updateSpace,this),
        static_cast<int>(updateOrder::spaceUpdate)
    );
}

GSpaceScene::~GSpaceScene()
{
    delete gspace;
}

void GSpaceScene::updateSpace()
{
    gspace->update();
}

void GSpaceScene::processAdditions()
{
    gspace->processAdditions();
}

MapScene::MapScene(const string& res) :
mapRes("maps/"+res+".tmx")
{
    multiInit.insertWithOrder(
        wrap_method(MapScene,loadMap,this),
        static_cast<int>(initOrder::mapLoad)
    );
}

SpaceVect MapScene::getMapSize()
{
    return toChipmunk(tileMap->getMapSize());
}


void MapScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        ValueMap& objAsMap = obj.asValueMap();
        convertToUnitSpace(objAsMap);
        gspace->addObject(objAsMap);
    }
}

void MapScene::loadWalls()
{
    TMXObjectGroup* walls = tileMap->getObjectGroup("walls");
    if(!walls)
        return;
    
    foreach(Value obj, walls->getObjects())
    {
        ValueMap& objAsMap = obj.asValueMap();
        cocos2d::Rect area = getUnitspaceRectangle(objAsMap);
        gspace->addWallBlock(toChipmunk(area.origin), toChipmunk(area.getUpperCorner()));
    }
}


void MapScene::loadMapObjects(const TMXTiledMap& map)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"));
    }
}

void MapScene::loadPaths(const TMXTiledMap& map)
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
		SpaceVect origin(asMap.at("x").asFloat(), asMap.at("y").asFloat());

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

void MapScene::loadMap()
{
    tileMap = TMXTiledMap::create(mapRes);
    
    if(tileMap)
        log("Map %s loaded.", mapRes.c_str());
    else
        log("Map %s not found.", mapRes.c_str());
    
    getLayer(sceneLayers::space)->addChild(
        tileMap,
        static_cast<int>(GraphicsLayer::map)
    );
	loadPaths(*tileMap);
    loadMapObjects(*tileMap);
    
    cocos2d::Size size = tileMap->getMapSize();
    gspace->setSize(size.width, size.height);
    
    loadWalls();
    gspace->addWallBlock(SpaceVect(-1,0), SpaceVect(0,size.height));
    gspace->addWallBlock(SpaceVect(size.width,0), SpaceVect(size.width+1,size.height));
    gspace->addWallBlock(SpaceVect(0,size.height), SpaceVect(size.width,size.height+1));
    gspace->addWallBlock(SpaceVect(0,-1), SpaceVect(size.width,0));
}

ScriptedScene::ScriptedScene(const string& res) :
ctx(make_unique<Lua::Inst>("scene"))
{
    multiInit.insertWithOrder(
        wrap_method(ScriptedScene, runInit,this),
        static_cast<int>(initOrder::postLoadObjects)
    );
    multiUpdate.insertWithOrder(
        wrap_method(ScriptedScene,runUpdate,this),
        static_cast<int>(updateOrder::sceneUpdate)
    );
    
    if(!res.empty())
    {
        string path = "scripts/scenes/"+res+".lua";
    
        if(!FileUtils::getInstance()->isFileExist(path))
            log("ScriptedScene: %s script does not exist.", res.c_str());
        else
            ctx->runFile(path);
    }
}


PlayScene* GScene::playScene(){
    return dynamic_cast<PlayScene*>(crntScene);
}

GSpace* GScene::getSpace()
{
    GSpaceScene* scene = dynamic_cast<GSpaceScene*>(crntScene);
    
    if(scene) return scene->gspace;
    else return nullptr;
}

HUD* GScene::getHUD()
{
    PlayScene* ps = playScene();
    
    if(ps) return ps->hud;
    else return nullptr;
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
        (-App::pixelsPerTile*v.x+App::width/2)*spaceZoom,
        (-App::pixelsPerTile*v.y+App::height/2)*spaceZoom
    );
}

void GScene::createDialog(const string& res, bool autoAdvance)
{
    dialogNode = Dialog::create();
    dialogNode->setDialog(res);
    dialogNode->setPosition(dialogPosition());
    getLayer(sceneLayers::dialog)->addChild(dialogNode);
    
    //This options are not actually mutually exclusive, but for simplicity just use a flag
    //to choose one.
    dialogNode->setAutoAdvance(autoAdvance);
    dialogNode->setManualAdvance(!autoAdvance);
    
    dialogNode->setEndHandler([=]() -> void {
        getLayer(sceneLayers::dialog)->removeChild(dialogNode);
        dialogNode = nullptr;
    });
}

void GScene::stopDialog()
{
    if(dialogNode){
        getLayer(sceneLayers::dialog)->removeChild(dialogNode);
        dialogNode = nullptr;
    }
}

void ScriptedScene::runInit()
{
    ctx->callIfExistsNoReturn("init");
}
void ScriptedScene::runUpdate()
{
    ctx->callIfExistsNoReturn("update");
}
