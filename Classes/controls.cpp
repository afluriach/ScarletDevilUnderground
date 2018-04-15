//
//  controls.cpp
//  Koumachika
//
//  Created by Toni on 10/15/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"

//Backtick "KEY_TILDE" enum does not work on OS X.
const EventKeyboard::KeyCode backtickKey = static_cast<EventKeyboard::KeyCode>(123);
//Similarly, neither KEY_RETURN nor KEY_ENTER are recognized
const EventKeyboard::KeyCode returnKey = static_cast<EventKeyboard::KeyCode>(35);

const KeyCodeMap watchedKeys = boost::assign::map_list_of
(EventKeyboard::KeyCode::KEY_Z, Keys::action)
(backtickKey, Keys::backtick)
(EventKeyboard::KeyCode::KEY_1, Keys::num1)
(returnKey, Keys::enter)
(EventKeyboard::KeyCode::KEY_ESCAPE, Keys::escape)
(EventKeyboard::KeyCode::KEY_W, Keys::moveUp)
(EventKeyboard::KeyCode::KEY_S, Keys::moveDown)
(EventKeyboard::KeyCode::KEY_A, Keys::moveLeft)
(EventKeyboard::KeyCode::KEY_D, Keys::moveRight)
(EventKeyboard::KeyCode::KEY_UP_ARROW, Keys::arrowUp)
(EventKeyboard::KeyCode::KEY_DOWN_ARROW, Keys::arrowDown)
(EventKeyboard::KeyCode::KEY_LEFT_ARROW, Keys::arrowLeft)
(EventKeyboard::KeyCode::KEY_RIGHT_ARROW, Keys::arrowRight);

KeyRegister::KeyRegister()
{
    //Initialize key held map by putting each key enum in it.
    for(auto it = watchedKeys.begin(); it != watchedKeys.end(); ++it)
    {
        keyDown[it->second] = false;
    }
    
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = bindMethod(&KeyRegister::onKeyDown, this);
    keyListener->onKeyReleased = bindMethod(&KeyRegister::onKeyUp, this);

    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
        keyListener,
        static_cast<int>(App::EventPriorities::KeyRegisterEvent)
    );
}

#define MOVE_KEYS isKeyDown(Keys::moveUp), isKeyDown(Keys::moveDown), isKeyDown(Keys::moveLeft), isKeyDown(Keys::moveRight)
#define ARROW_KEYS isKeyDown(Keys::arrowUp), isKeyDown(Keys::arrowDown), isKeyDown(Keys::arrowLeft), isKeyDown(Keys::arrowRight)

Vec2 getDirectionVecFromKeyQuad(bool up, bool down, bool left, bool right)
{
    Vec2 result;

    if(up && !down) result.y = 1;
    if(down && !up) result.y = -1;
    if(left && !right) result.x = -1;
    if(right && !left) result.x = 1;
    
    if(result.lengthSquared() > 1)
        result.normalize();
    
    return result;
}

Vec2 KeyRegister::getMoveKeyState()
{
    return getDirectionVecFromKeyQuad(MOVE_KEYS);
}

Vec2 KeyRegister::getArrowKeyState()
{
    return getDirectionVecFromKeyQuad(ARROW_KEYS);
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
    Director::getInstance()->getEventDispatcher()->addEventListenerWithFixedPriority(
        keyListener,
        static_cast<int>(App::EventPriorities::KeyGlobalListenerEvent)
    );
}

KeyListener::KeyListener(Node* node)
{
    initListener();
    node->getEventDispatcher()->addEventListenerWithSceneGraphPriority(keyListener, node);
}

void KeyListener::initListener()
{
    keyListener = EventListenerKeyboard::create();
    keyListener->onKeyPressed = bindMethod(&KeyListener::onKeyDown, this);
    keyListener->onKeyReleased = bindMethod(&KeyListener::onKeyUp, this);
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
