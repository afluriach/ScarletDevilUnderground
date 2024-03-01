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

class PlayScene : public GScene
{
public:
    static const Color3B fadeoutColor;
    static const float fadeoutLength;

	PlayScene(shared_ptr<area_properties> props, string start);

	virtual ~PlayScene();

	virtual void update(float dt);

	void loadPlayer();
	void showVisibleRooms();

	void onPausePressed();
	virtual void enterPause();
	virtual void exitPause();
    
	void onMapPressed();
	void enterMap();
	void exitMap();
	
	void onInventoryPressed();
	void enterInventoryMenu();
	void exitInventoryMenu();

	void enterWorldSelect();
	void exitWorldSelect();

    void pauseAnimations();
    void resumeAnimations();
    
    void triggerGameOver();
    void showGameOverMenu();

	virtual GScene* getReplacementScene();

	inline void setIsOverworld(bool val) { isOverworld = val; }

    HUD* hud = nullptr;
	PauseMenu* pauseMenu = nullptr;
	MapMenu* mapMenu = nullptr;
	InventoryInfo* inventoryMenu = nullptr;

private:    
    void addHUD();
	void showMenu(MenuLayer* menu);
	void triggerMenu( void (PlayScene::*m)(void) );

	bool isShowingMenu = false;
	bool isOverworld = false;
};

template<typename...Args>
inline zero_arity_function make_hud_action(void (HUD::*m)(Args...), PlayScene* playScene, Args ...args)
{
	return generate_action(playScene->hud, m, args...);
}

#endif /* PlayScene_hpp */
