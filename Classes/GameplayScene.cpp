//
//  GameplayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include <boost/assign.hpp>
#include "GameplayScene.hpp"
#include "controls.h"

USING_NS_CC;

bool GameplayScene::init()
{
    scheduleUpdate();
    
    tileMap = TMXTiledMap::create("maps/floor1_start.tmx");
    
    addChild(tileMap);
    
    log("Gameplay scene initialized.");
    return true;
}

void GameplayScene::update(float dt)
{
    KeyRegister* kr = KeyRegister::inst();
    
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
