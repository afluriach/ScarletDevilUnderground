//
//  PlayScene.hpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include "HUD.hpp"
#include "menu.h"
#include "scenes.h"

class PauseMenu;

class PlayScene : virtual public GScene, public MapScene, public ScriptedScene
{
public:    
    inline PlayScene(const string& name) : MapScene(name), ScriptedScene(name)
    {
        multiInit.insertWithOrder(
            wrap_method(PlayScene,trackPlayer,this),
            static_cast<int>(initOrder::postLoadObjects)
        );
        multiInit.insertWithOrder(
            wrap_method(PlayScene,addHUD,this),
            static_cast<int>(initOrder::initHUD)
        );
        multiUpdate.insertWithOrder(
            wrap_method(PlayScene,updateCamera,this),
            static_cast<int>(updateOrder::moveCamera)
        );

		keyListener.addPressListener(
			Keys::escape,
			[=]()-> void {onPausePressed(); }
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

	void onPausePressed();
	void enterPause();
	void exitPause();
    
    void pauseAnimations();
    void resumeAnimations();
    
    HUD* hud;
	PauseMenu* pauseMenu;
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
