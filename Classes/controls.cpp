//
//  controls.cpp
//  FlansBasement
//
//  Created by Toni on 10/15/15.
//
//

#include "App.h"
#include "controls.h"
#include "util.h"

using namespace std::placeholders;
USING_NS_CC;

//Backtick "KEY_TILDE" enum does not work on OS X.
const EventKeyboard::KeyCode backtickKey = static_cast<EventKeyboard::KeyCode>(123);
//Similarly, neither KEY_RETURN nor KEY_ENTER are recognized
const EventKeyboard::KeyCode returnKey = static_cast<EventKeyboard::KeyCode>(35);

const KeyCodeMap watchedKeys = map_list_of
(EventKeyboard::KeyCode::KEY_Z, Keys::action)
(backtickKey, Keys::backtick)
(returnKey, Keys::enter)
(EventKeyboard::KeyCode::KEY_UP_ARROW, Keys::up)
(EventKeyboard::KeyCode::KEY_DOWN_ARROW, Keys::down)
(EventKeyboard::KeyCode::KEY_LEFT_ARROW, Keys::left)
(EventKeyboard::KeyCode::KEY_RIGHT_ARROW, Keys::right);

KeyRegister::KeyRegister()
{
    //Initialize key held map by putting each key enum in it.
    for(auto it = watchedKeys.begin(); it != watchedKeys.end(); ++it)
    {
        keyDown[it->second] = false;
    }
    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = std::bind(&KeyRegister::onKeyDown, this, _1, _2);
    keyListener->onKeyReleased = std::bind(&KeyRegister::onKeyUp, this, _1, _2);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(keyListener, App::EventPriorities::KeyRegisterEvent);
}

void KeyRegister::onKeyDown(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d pressed", code);
    
    if(watchedKeys.find(code) != watchedKeys.end())
    {
        keyDown[watchedKeys.find(code)->second] = true;
    }
}

void KeyRegister::onKeyUp(EventKeyboard::KeyCode code, Event* event)
{
    if(logKeyEvents)
        log("%d released", code);
    
    if(watchedKeys.find(code) != watchedKeys.end())
    {
        keyDown[watchedKeys.find(code)->second] = false;
    }
}

bool KeyRegister::isKeyDown(const Keys& key)
{
    auto result = keyDown.find(key);
    
    if(result == keyDown.end())
    {
        log("isKeyDown: warning, unknown enum value %d.", key);
        return false;
    }
    
    return result->second;
}

KeyListener::KeyListener()
{
    initListener();
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(keyListener, App::EventPriorities::KeyGlobalListenerEvent);
}

KeyListener::KeyListener(cocos2d::Node* node)
{
    initListener();
    node->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, node);
}

void KeyListener::initListener()
{
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = std::bind(&KeyListener::onKeyDown, this, _1, _2);
    keyListener->onKeyReleased = std::bind(&KeyListener::onKeyUp, this, _1, _2);
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
