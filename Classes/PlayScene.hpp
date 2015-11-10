//
//  PlayScene.hpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include <map>

#include "cocos2d.h"

#include "controls.h"
#include "GSpace.hpp"

class PlayScene : public cocos2d::Layer
{
public:
    inline PlayScene() : gspace(this) {}
    
    enum Layer{
        map = 1,
        ground,
    };
    
    virtual bool init();
    virtual void update(float dt);
    
    CREATE_FUNC(PlayScene);
    
    static PlayScene* inst;
    
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

#endif /* PlayScene_hpp */
