//
//  PlayScene.hpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef PlayScene_hpp
#define PlayScene_hpp

#include "functional.hpp"
#include "GScene.hpp"

class GObject;
class HUD;
class MapMenu;
class MenuLayer;
class PauseMenu;

class PlayScene : public GScene
{
public:
	typedef function<PlayScene* () > AdapterType;

    static const Color3B fadeoutColor;
    static const float fadeoutLength;

    PlayScene(const string& name);
	PlayScene(const string& sceneName, const vector<MapEntry>& maps);

	virtual ~PlayScene();

	virtual void update(float dt);

	void showVisibleRooms();

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

	bool isShowingMenu = false;
	bool isOverworld = false;
};

template<typename...Args>
inline zero_arity_function make_hud_action(void (HUD::*m)(Args...), PlayScene* playScene, Args ...args)
{
	return generate_action(playScene->hud, m, args...);
}

#endif /* PlayScene_hpp */
