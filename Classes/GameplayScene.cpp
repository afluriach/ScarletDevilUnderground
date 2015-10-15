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
using namespace std::placeholders;

typedef std::map<EventKeyboard::KeyCode, Keys> KeyCodeMap;

const KeyCodeMap GameplayScene::keys = boost::assign::map_list_of
(EventKeyboard::KeyCode::KEY_UP_ARROW, Keys::up)
(EventKeyboard::KeyCode::KEY_DOWN_ARROW, Keys::down)
(EventKeyboard::KeyCode::KEY_LEFT_ARROW, Keys::left)
(EventKeyboard::KeyCode::KEY_RIGHT_ARROW, Keys::right);

void GameplayScene::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    log("%d pressed", code);
    
    if(keys.find(code) != keys.end())
    {
        keyHeld[keys.find(code)->second] = true;
    }
}

void GameplayScene::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    log("%d released", code);
    
    if(keys.find(code) != keys.end())
    {
        keyHeld[keys.find(code)->second] = false;
    }
}


bool GameplayScene::init()
{
    //Initialize key held map by making sure each key name is in it.
    for(auto it = keys.begin(); it != keys.end(); ++it)
    {
        keyHeld[it->second] = false;
    }
    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = std::bind(&GameplayScene::onKeyDown, this, _1, _2);
    keyListener->onKeyReleased = std::bind(&GameplayScene::onKeyUp, this, _1, _2);
    getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, this);
    
    scheduleUpdate();
    
    tileMap = TMXTiledMap::create("maps/floor1_start.tmx");
    
    addChild(tileMap);
    
    log("Gameplay scene initialized.");
    return true;
}

void GameplayScene::update(float dt)
{
    //Check camera scroll.
    if(isKeyHeld(Keys::up) && !isKeyHeld(Keys::down))
        move(0, cameraMovePixPerFrame);
    if(isKeyHeld(Keys::down) && !isKeyHeld(Keys::up))
        move(0, -cameraMovePixPerFrame);
    if(isKeyHeld(Keys::left) && !isKeyHeld(Keys::right))
        move(-cameraMovePixPerFrame, 0);
    if(isKeyHeld(Keys::right) && !isKeyHeld(Keys::left))
        move(cameraMovePixPerFrame, 0);
    
}

void GameplayScene::move(int dx, int dy)
{
    setPosition(getPositionX()-dx, getPositionY()-dy);
}

bool GameplayScene::isKeyHeld(const Keys& key)
{
    auto result = keyHeld.find(key);
    
    if(result == keyHeld.end())
    {
        log("isKeyHeld: warning, unknown enum value %d.", key);
        return false;
    }
    
    return result->second;
}