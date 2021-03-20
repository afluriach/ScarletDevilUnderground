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

    PlayScene(const string& name, const string& start);
	PlayScene(area_properties props, const string& start);

	virtual ~PlayScene();

	virtual void update(float dt);

	void loadPlayer();
	void showVisibleRooms();

	void onPausePressed();
	virtual void enterPause();
	virtual void exitPause();
    
	inline virtual string getPlayerStart() const { return start; }
	virtual Color4F getDefaultAmbientLight() const;

	void onMapPressed();
	void enterMap();
	void exitMap();

	void enterWorldSelect();
	void exitWorldSelect();

    void pauseAnimations();
    void resumeAnimations();
    
    void triggerGameOver();
    void showGameOverMenu();

	virtual GScene* getReplacementScene();

	inline virtual string getNextLevel() const { return props.next; }
	inline void setIsOverworld(bool val) { isOverworld = val; }

    HUD* hud = nullptr;
	PauseMenu* pauseMenu = nullptr;
	MapMenu* mapMenu = nullptr;

private:    
    void addHUD();
	void showMenu(MenuLayer* menu);
	void triggerMenu( void (PlayScene::*m)(void) );

	area_properties props;
	string start;

	bool isShowingMenu = false;
	bool isOverworld = false;
};

template<typename...Args>
inline zero_arity_function make_hud_action(void (HUD::*m)(Args...), PlayScene* playScene, Args ...args)
{
	return generate_action(playScene->hud, m, args...);
}

#endif /* PlayScene_hpp */
