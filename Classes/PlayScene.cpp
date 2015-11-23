//
//  PlayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include "App.h"
#include "controls.h"
#include "PlayScene.hpp"
#include "GObject.hpp"
#include "util.h"

using namespace std;
USING_NS_CC;

PlayScene* PlayScene::inst;

bool PlayScene::init()
{
    inst = this;
    
    scheduleUpdate();
    
    tileMap = TMXTiledMap::create("maps/block_room.tmx");
    
    addChild(tileMap, Layer::map);
    
    loadMapObjects(*tileMap);
    
    log("Gameplay scene initialized.");
    return true;
}

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        printValueMap(objAsMap);
    }
}

void PlayScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    gspace.addObjects(objects);
    //This isn't really necessary. If this is not added, nothing will happen the first frame
    //since there will be no objects, and all objects will be added at the end of the first frame.
    gspace.processAdditions();
}

void PlayScene::loadMapObjects(const TMXTiledMap& map)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"));
    }
}

void PlayScene::update(float dt)
{
    KeyRegister* kr = app->keyRegister;
    
    //Check camera scroll.
    if(kr->isKeyDown(Keys::up) && !kr->isKeyDown(Keys::down))
        move(0, cameraMovePixPerFrame);
    if(kr->isKeyDown(Keys::down) && !kr->isKeyDown(Keys::up))
        move(0, -cameraMovePixPerFrame);
    if(kr->isKeyDown(Keys::left) && !kr->isKeyDown(Keys::right))
        move(-cameraMovePixPerFrame, 0);
    if(kr->isKeyDown(Keys::right) && !kr->isKeyDown(Keys::left))
        move(cameraMovePixPerFrame, 0);
    
    gspace.update();
}

void PlayScene::move(int dx, int dy)
{
    setPosition(getPositionX()-dx, getPositionY()-dy);
}
