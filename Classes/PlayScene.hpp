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

class PlayScene : virtual public GScene, public MapScene
{
public:    
    inline PlayScene(const string& map) : MapScene(map)
    {
        multiInit.insertWithOrder(bind(&PlayScene::trackPlayer, this), initOrder::postLoadObjects);
        multiUpdate.insertWithOrder(bindMethod(&PlayScene::updateCamera, this), updateOrder::moveCamera);
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
private:    
    const int cameraMovePixPerFrame = 3;
    GObject* cameraTarget = nullptr;
};

#endif /* PlayScene_hpp */
