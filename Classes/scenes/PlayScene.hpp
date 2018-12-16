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
#include "menu_layers.h"
#include "scenes.h"

class GObject;
class PauseMenu;

class PlayScene : public GScene
{
public:
    static const Color3B fadeoutColor;
    static const float fadeoutLength;

    PlayScene(const string& name);
	PlayScene(const string& sceneName, const vector<MapEntry>& maps);

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
    void showGameOverMenu();

	void triggerSceneCompleted();
	void showSceneCompletedMenu();

	void triggerReplayCompleted();
	void showReplayCompletedMenu();

	virtual GScene* getReplacementScene();

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

#endif /* PlayScene_hpp */
