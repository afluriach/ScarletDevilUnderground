//
//  scenes.cpp
//  Koumachika
//
//  Created by Toni on 11/23/15.
//
//

#include "Prefix.h"

GScene* GScene::crntScene;

void GScene::runScene(const string& name)
{
    auto it = adapters.find(name);
    
    if(it == adapters.end()){
        throw runtime_error("runScene: " + name + " not found.");
    }
    else
    {
        it->second();
    }
}

GScene::GScene()
{
    //Updater has to be scheduled at init time.
    multiInit.insertWithOrder(wrap_method(GScene,initUpdate), initOrder::core);

    crntScene = this;
    
    //Create the sublayers at construction (so they are available to mixins at construction time).
    //But do not add sublayers until init time.
    for(int i=1; i<sceneLayers::nLayers; ++i){
        Layer* l = Layer::create();
        layers.insert(i, l);
    }
}

bool GScene::init()
{
    Layer::init();
    
    for(int i=1; i<sceneLayers::nLayers; ++i){
        Layer* l = layers.at(i);
        Node::addChild(l,i);
    }
    
    //Apply zoom to adjust viewable area size.
    float baseViewWidth = App::width * App::tilesPerPixel;
    spaceZoom = baseViewWidth / App::viewWidth;
    //Only apply zoom to space layer.
    getLayer(sceneLayers::spaceLayer)->setScale(spaceZoom);
    
    multiInit(this);
    
    return true;
}

void GScene::update(float dt)
{
    if(!isPaused)
        multiUpdate(this);
    else
    {
        app->checkPendingScript();
    }
}

void GScene::setPaused(bool p){
    isPaused = p;
}

void MapScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        ValueMap& objAsMap = obj.asValueMap();
        convertToUnitSpace(objAsMap);
        gspace.addObject(objAsMap);
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

void MapScene::loadMap()
{
    tileMap = TMXTiledMap::create(mapRes);
    
    if(tileMap)
        log("Map %s loaded.", mapRes.c_str());
    else
        log("Map %s not found.", mapRes.c_str());
    
    getLayer(sceneLayers::spaceLayer)->addChild(tileMap, GraphicsLayer::map);
    loadMapObjects(*tileMap);
    
    cocos2d::Size size = tileMap->getMapSize();
    gspace.setSize(size.width, size.height);
    
    gspace.addWallBlock(SpaceVect(-1,0), SpaceVect(0,size.height));
    gspace.addWallBlock(SpaceVect(size.width,0), SpaceVect(size.width+1,size.height));
    gspace.addWallBlock(SpaceVect(0,size.height), SpaceVect(size.width,size.height+1));
    gspace.addWallBlock(SpaceVect(0,-1), SpaceVect(size.width,0));
}

GSpace* GScene::getSpace()
{
    GSpaceScene* scene = dynamic_cast<GSpaceScene*>(crntScene);
    
    if(scene) return &(scene->gspace);
    else return nullptr;
}

void GScene::move(const Vec2& w)
{
    Vec2 v = w * spaceZoom;
    getLayer(sceneLayers::spaceLayer)->setPosition(getPositionX()-v.x, getPositionY()-v.y);
}

void GScene::setUnitPosition(const SpaceVect& v)
{
    getLayer(sceneLayers::spaceLayer)->setPosition(
        (-App::pixelsPerTile*v.x+App::width/2)*spaceZoom,
        (-App::pixelsPerTile*v.y+App::height/2)*spaceZoom
    );
}

void GScene::createDialog(const string& res, bool autoAdvance)
{
    dialogNode = Dialog::create();
    dialogNode->setDialog(res);
    dialogNode->setPosition(dialogPosition());
    getLayer(sceneLayers::dialogLayer)->addChild(dialogNode);
    
    //This options are not actually mutually exclusive, but for simplicity just use a flag
    //to choose one.
    dialogNode->setAutoAdvance(autoAdvance);
    dialogNode->setManualAdvance(!autoAdvance);
    
    dialogNode->setEndHandler([=]() -> void {
        getLayer(sceneLayers::dialogLayer)->removeChild(dialogNode);
        dialogNode = nullptr;
    });
}

void GScene::stopDialog()
{
    if(dialogNode){
        getLayer(sceneLayers::dialogLayer)->removeChild(dialogNode);
        dialogNode = nullptr;
    }
}