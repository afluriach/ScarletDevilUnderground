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

	void initReplayData();
	void updateReplayData();
	bool loadReplayData(const string& filename);
	void saveReplayData(const string& filename);
	ControlInfo getControlData();

    void updateHUD();
    void applyCameraControls();

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

	unique_ptr<ControlReplay> controlReplay;
	bool isRunningReplay = false;
	bool isShowingMenu = false;
};

template<typename...Args>
inline pair<function<void(void)>, GScene::updateOrder> make_hud_action(void (HUD::*m)(Args...), PlayScene* playScene, Args ...args)
{
	return pair<function<void(void)>, GScene::updateOrder>(generate_action(playScene->hud, m, args...), GScene::updateOrder::hudUpdate);
}

#endif /* PlayScene_hpp */
