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
#include "scenes.h"

class GObject;
class MapMenu;
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
	void setSuppressAction(bool val);
	bool getSuppressAction();

	void onPausePressed();
	virtual void enterPause();
	virtual void exitPause();
    
	void onMapPressed();
	void enterMap();
	void exitMap();

    void pauseAnimations();
    void resumeAnimations();
    
    void triggerGameOver();
    void showGameOverMenu();
	inline bool getGameOver() { return isGameOver; }

	void triggerSceneCompleted();
	void showSceneCompletedMenu();

	void triggerReplayCompleted();
	void showReplayCompletedMenu();

	virtual GScene* getReplacementScene();

    HUD* hud = nullptr;
	PauseMenu* pauseMenu = nullptr;
	MapMenu* mapMenu = nullptr;

private:    
    void addHUD();
	void showMenu(Layer* menu);
	void triggerMenu( void (PlayScene::*m)(void) );

	SpaceFloat cameraMoveTilesPerSecond = 3.0;
    gobject_ref cameraTarget;

	unique_ptr<ControlReplay> controlReplay;
	bool isRunningReplay = false;
	bool isShowingMenu = false;
	bool isSuppressAction = false;
	bool isGameOver = false;
};

#endif /* PlayScene_hpp */
