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

#include "HUD.hpp"

class PlayScene : virtual public GScene, public MapScene
{
public:    
    inline PlayScene(const string& map) : MapScene(map)
    {
        multiInit.insertWithOrder(wrap<PlayScene,&PlayScene::trackPlayer>(), initOrder::postLoadObjects);
        multiInit.insertWithOrder(wrap<PlayScene,&PlayScene::addHUD>(), initOrder::postLoadObjects);
        multiUpdate.insertWithOrder(wrap<PlayScene,&PlayScene::updateCamera>(), updateOrder::moveCamera);
    }
    
    //set player object. should be post load object init
    inline void trackPlayer(){
        cameraTarget = gspace.getObject("player");
    }
    
    void updateHUD();
    void updateCamera();
    void applyCameraControls();
    void trackCameraTarget();
    
    inline void setCameraTarget(GObject* target){
        cameraTarget = target;
    }
private:    
    void addHUD();

    const int cameraMovePixPerFrame = 3;
    GObject* cameraTarget = nullptr;
    HUD* hud;
};

#endif /* PlayScene_hpp */
