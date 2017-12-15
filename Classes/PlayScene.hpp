//
//  PlayScene.hpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

//Already included in PCH
//#include "scenes.h"

#include "HUD.hpp"

class PlayScene : virtual public GScene, public MapScene, public ScriptedScene
{
public:    
    inline PlayScene(const string& name) : MapScene(name), ScriptedScene(name)
    {
        multiInit.insertWithOrder(
            wrap_method(PlayScene,trackPlayer),
            static_cast<int>(initOrder::postLoadObjects)
        );
        multiInit.insertWithOrder(
            wrap_method(PlayScene,addHUD),
            static_cast<int>(initOrder::initHUD)
        );
        multiUpdate.insertWithOrder(
            wrap_method(PlayScene,updateCamera),
            static_cast<int>(updateOrder::moveCamera)
        );
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
    
    HUD* hud;
private:    
    void addHUD();

    const int cameraMovePixPerFrame = 3;
    GObject* cameraTarget = nullptr;
};

#define GenericPlayScene( name ) \
class name : public PlayScene \
{ \
public: \
    CREATE_FUNC(name); \
    inline name() : PlayScene(#name){ \
    } \
};

#endif /* PlayScene_hpp */
