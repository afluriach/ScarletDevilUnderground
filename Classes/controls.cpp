//
//  controls.cpp
//  FlansBasement
//
//  Created by Toni on 10/15/15.
//
//

#include <boost/assign.hpp>

#include "AppDelegate.h"
#include "controls.h"

using namespace std::placeholders;
USING_NS_CC;

const KeyCodeMap watchedKeys = boost::assign::map_list_of
(EventKeyboard::KeyCode::KEY_Z, Keys::action)
(EventKeyboard::KeyCode::KEY_UP_ARROW, Keys::up)
(EventKeyboard::KeyCode::KEY_DOWN_ARROW, Keys::down)
(EventKeyboard::KeyCode::KEY_LEFT_ARROW, Keys::left)
(EventKeyboard::KeyCode::KEY_RIGHT_ARROW, Keys::right);

KeyRegister* KeyRegister::_inst;


KeyRegister::KeyRegister()
{
    //Initialize key held map by putting each key enum in it.
    for(auto it = watchedKeys.begin(); it != watchedKeys.end(); ++it)
    {
        keyHeld[it->second] = false;
    }
    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = std::bind(&KeyRegister::onKeyDown, this, _1, _2);
    keyListener->onKeyReleased = std::bind(&KeyRegister::onKeyUp, this, _1, _2);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(keyListener, AppDelegate::EventPriorities::KeyRegister);
}

void KeyRegister::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    log("%d pressed", code);
    
    if(watchedKeys.find(code) != watchedKeys.end())
    {
        keyHeld[watchedKeys.find(code)->second] = true;
    }
}

void KeyRegister::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    log("%d released", code);
    
    if(watchedKeys.find(code) != watchedKeys.end())
    {
        keyHeld[watchedKeys.find(code)->second] = false;
    }
}

bool KeyRegister::isKeyHeld(const Keys& key)
{
    auto result = keyHeld.find(key);
    
    if(result == keyHeld.end())
    {
        log("isKeyHeld: warning, unknown enum value %d.", key);
        return false;
    }
    
    return result->second;
}

KeyListener::KeyListener(cocos2d::Node* node)
{
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = std::bind(&KeyListener::onKeyDown, this, _1, _2);
    keyListener->onKeyReleased = std::bind(&KeyListener::onKeyUp, this, _1, _2);
    
    node->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, node);
}

void KeyListener::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    auto code_it = watchedKeys.find(code);
    
    //If this is a watched key.
    if(code_it != watchedKeys.end())
    {
        Keys k = code_it->second;
        
        auto key_it  = onPressed.find(k);
        
        //if there is a callback for this key.
        if(key_it != onPressed.end())
        {
            key_it->second();
        }
    }
}

void KeyListener::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    auto code_it = watchedKeys.find(code);
    
    //If this is a watched key.
    if(code_it != watchedKeys.end())
    {
        Keys k = code_it->second;
        
        auto key_it  = onReleased.find(k);
        
        //if there is a callback for this key.
        if(key_it != onReleased.end())
        {
            key_it->second();
        }
    }

}
