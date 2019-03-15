//
//  PlayScene.hpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include "GScene.hpp"

class GObject;
class HUD;
class MapMenu;
class MenuLayer;
class PauseMenu;
class Replay;

class PlayScene : public GScene
{
public:
	typedef function<PlayScene* () > AdapterType;

	static const unordered_map<ChamberID, AdapterType> adapters;
    static const Color3B fadeoutColor;
    static const float fadeoutLength;

	static PlayScene* runScene(ChamberID id);

    PlayScene(const string& name);
	PlayScene(const string& sceneName, const vector<MapEntry>& maps);

	virtual ~PlayScene();

	virtual void update(float dt);

	bool loadReplayData(const string& filename);
	bool loadReplayData(unique_ptr<Replay> _replay);
	void saveReplayData(const string& filename);
	void autosaveReplayData();

    void applyCameraControls();

	void onPausePressed();
	virtual void enterPause();
	virtual void exitPause();
    
	void onMapPressed();
	void enterMap();
	void exitMap();

	void enterWorldSelect();
	void exitWorldSelect();

    void pauseAnimations();
    void resumeAnimations();
    
    void triggerGameOver();
    void showGameOverMenu();

	void triggerSceneCompleted();
	void showSceneCompletedMenu();

	void triggerReplayCompleted();
	void showReplayCompletedMenu();

	virtual GScene* getReplacementScene();

	inline void setIsOverworld(bool val) { isOverworld = val; }

    HUD* hud = nullptr;
	PauseMenu* pauseMenu = nullptr;
	MapMenu* mapMenu = nullptr;

private:    
    void addHUD();
	void showMenu(MenuLayer* menu);
	void triggerMenu( void (PlayScene::*m)(void) );

	SpaceFloat cameraMoveTilesPerSecond = 3.0;

	bool isRunningReplay = false;
	bool isShowingMenu = false;
	bool isOverworld = false;
};

template<typename...Args>
inline zero_arity_function make_hud_action(void (HUD::*m)(Args...), PlayScene* playScene, Args ...args)
{
	return generate_action(playScene->hud, m, args...);
}

#endif /* PlayScene_hpp */
