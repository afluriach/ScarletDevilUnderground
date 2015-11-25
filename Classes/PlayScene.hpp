//
//  PlayScene.hpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include "Prefix.h"

#include "controls.h"
#include "scenes.h"

class PlayScene : virtual public GScene, MapScene
{
public:    
    inline PlayScene() : MapScene("maps/block_room.tmx")
    {
        addUpdate(bind(&PlayScene::updateCamera, this, placeholders::_1));
    }
    
    virtual void updateCamera(float dt);
    
    CREATE_FUNC(PlayScene);    
private:    
    const int cameraMovePixPerFrame = 3;
    void move(const cocos2d::Vec2& v);
};

#endif /* PlayScene_hpp */
