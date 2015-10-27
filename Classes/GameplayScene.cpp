//
//  GameplayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include <boost/assign.hpp>
#include "GameplayScene.hpp"
#include "util.h"
#include "controls.h"
#include "AppDelegate.h"

USING_NS_CC;

bool GameplayScene::init()
{
    scheduleUpdate();
    
    tileMap = TMXTiledMap::create("maps/block_room.tmx");
    
    addChild(tileMap);
    
    loadMapObjects(*tileMap);
    
    log("Gameplay scene initialized.");
    return true;
}

void printObject(const ValueMap& obj)
{
    foreach(ValueMap::value_type entryPair, obj)
    {
        log(
            "Object property %s: %s.",
            entryPair.first.c_str(),
            entryPair.second.asString().c_str()
            );
    }
}

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        
        foreach(ValueMap::value_type entryPair, objAsMap)
        {
            log(
                "Object property %s: %s.",
                entryPair.first.c_str(),
                entryPair.second.asString().c_str()
                );
        }
    }
}

void GameplayScene::loadMapObjects(const TMXTiledMap& map)
{
    Vector<TMXObjectGroup*> objLayers = map.getObjectGroups();
    log("%ld object groups.", objLayers.size());
    
    if(map.getObjectGroup("objects") == nullptr){
        log("Objects group missing.");
    }
    else{
        printGroup(map.getObjectGroup("objects"));
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
