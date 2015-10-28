//
//  GameplayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include "AppDelegate.h"
#include "controls.h"
#include "GameplayScene.hpp"
#include "GObject.hpp"
#include "util.h"

using namespace std;
USING_NS_CC;

GameplayScene* GameplayScene::inst;

bool GameplayScene::init()
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

void GameplayScene::loadObjectGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    gspace.addObjects(objects);
    gspace.processAdditions();
}

void GameplayScene::loadMapObjects(const TMXTiledMap& map)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    log("%ld object groups.", objLayers.size());
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        loadObjectGroup(map.getObjectGroup("objects"));
    }
}

void GameplayScene::update(float dt)
{
    KeyRegister* kr = app->keyRegister;
    
    //Check camera scroll.
    if(kr->isKeyHeld(Keys::up) && !kr->isKeyHeld(Keys::down))
        move(0, cameraMovePixPerFrame);
    if(kr->isKeyHeld(Keys::down) && !kr->isKeyHeld(Keys::up))
        move(0, -cameraMovePixPerFrame);
    if(kr->isKeyHeld(Keys::left) && !kr->isKeyHeld(Keys::right))
        move(-cameraMovePixPerFrame, 0);
    if(kr->isKeyHeld(Keys::right) && !kr->isKeyHeld(Keys::left))
        move(cameraMovePixPerFrame, 0);
    
}

void GameplayScene::move(int dx, int dy)
{
    setPosition(getPositionX()-dx, getPositionY()-dy);
}
