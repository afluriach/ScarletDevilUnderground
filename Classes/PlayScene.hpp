//
//  PlayScene.hpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

//Already included in PCH
//#include "scenes.h"

class PlayScene : virtual public GScene, MapScene
{
public:    
    inline PlayScene() : MapScene("maps/block_room.tmx")
    {
        multiInit.insertWithOrder(bind(&PlayScene::trackPlayer, this), initOrder::postLoadObjects);
        multiUpdate += bind(&PlayScene::updateCamera, this, placeholders::_1);
    }
    
    //set player object. should be post load object init
    inline void trackPlayer(){
        cameraTarget = gspace.getObjectByName("player");
    }
    
    void updateCamera(float dt);
    void applyCameraControls();
    void trackCameraTarget();
    
    inline void setCameraTarget(GObject* target){
        cameraTarget = target;
    }
    
    CREATE_FUNC(PlayScene);    
private:    
    const int cameraMovePixPerFrame = 3;
    GObject* cameraTarget = nullptr;
};

#endif /* PlayScene_hpp */
