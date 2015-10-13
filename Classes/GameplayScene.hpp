//
//  GameplayScene.hpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef GameplayScene_hpp
#define GameplayScene_hpp

#include "cocos2d.h"
#include <map>

class GameplayScene : public cocos2d::Layer
{
public:    
    virtual bool init();
    virtual void update(float dt);
    
    CREATE_FUNC(GameplayScene);
private:
    cocos2d::Layer* mapLayer;
    cocos2d::TMXTiledMap* tileMap;
    
    cocos2d::EventListenerKeyboard* keyListener;
    void onKeyPressed(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    void onKeyReleased(cocos2d::EventKeyboard::KeyCode, cocos2d::Event*);
    bool isKeyHeld(const std::string& key);
    
    //Map the key code to the string used to identify it
    static const std::map<cocos2d::EventKeyboard::KeyCode, std::string> keys;
    std::map<std::string, bool> keyHeld;
    
    const int cameraMovePixPerFrame = 3;
    void move(int dx, int dy);
};

#endif /* GameplayScene_hpp */
