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
    enum Layer{
        map = 1,
        ground,
    };
    
    virtual bool init();
    virtual void update(float dt);
    
    CREATE_FUNC(GameplayScene);
    
    static GameplayScene* inst;
    
    GSpace gspace;
private:
    cocos2d::Layer* mapLayer;
    cocos2d::TMXTiledMap* tileMap;
    
    void loadMapObjects(const cocos2d::TMXTiledMap& map);
    //Add a map object layer to space.
    void loadObjectGroup(TMXObjectGroup* group);
    
    const int cameraMovePixPerFrame = 3;
    void move(int dx, int dy);
};

#endif /* GameplayScene_hpp */
