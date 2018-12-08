//
//  PlayScene.hpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include "controls.h"
#include "object_ref.hpp"
#include "HUD.hpp"
#include "menu.h"
#include "scenes.h"

class GObject;
class PauseMenu;

class PlayScene : virtual public GScene, public MapScene, public ScriptedScene
{
public:
    static const Color3B fadeoutColor;
    static const float fadeoutLength;

    PlayScene(const string& name);

	virtual ~PlayScene();

    //set player object. should be post load object init
    void trackPlayer();

	void initReplayData();
	void updateReplayData();
	bool loadReplayData(const string& filename);
	bool saveReplayData(const string& filename);
	ControlInfo getControlData();

	void initRoomMask();
	void updateRoomMask();
    void updateHUD();
    void updateCamera();
    void applyCameraControls();
    void trackCameraTarget();

	void onPausePressed();
	virtual void enterPause();
	virtual void exitPause();
    
    void pauseAnimations();
    void resumeAnimations();
    
    void triggerGameOver();
    void showGameOverMenu(float unused);

	void triggerSceneCompleted();
	void showSceneCompletedMenu(float unused);

	void triggerReplayCompleted();
	void showReplayCompletedMenu(float unused);


    HUD* hud = nullptr;
	PauseMenu* pauseMenu = nullptr;
private:    
    void addHUD();

	vector<DrawNode*> roomMasks;

    const int cameraMovePixPerFrame = 3;
    gobject_ref cameraTarget;

	unique_ptr<ControlReplay> controlReplay;
	bool isRunningReplay = false;
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
