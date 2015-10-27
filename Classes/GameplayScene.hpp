//
//  GameplayScene.hpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef GameplayScene_hpp
#define GameplayScene_hpp

#include <map>

#include "cocos2d.h"

#include "controls.h"
#include "GSpace.hpp"

class GameplayScene : public cocos2d::Layer
{
public:    
    virtual bool init();
    virtual void update(float dt);
    
    CREATE_FUNC(GameplayScene);
private:
    cocos2d::Layer* mapLayer;
    cocos2d::TMXTiledMap* tileMap;
    
    GSpace gspace;
    
    void loadMapObjects(const cocos2d::TMXTiledMap& map);
    
    const int cameraMovePixPerFrame = 3;
    void move(int dx, int dy);
};

#endif /* GameplayScene_hpp */
