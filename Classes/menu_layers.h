//
//  menu_layers.h
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#ifndef menu_layers_h
#define menu_Layers_h

#include "menu.h"

class PlayScene;

class TitleMenu : public TextListMenuImpl<TitleMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline TitleMenu() {}
private:
	static void loadGame();
	static void sceneSelect();
};

class LoadProfileMenu : public TextListMenuImpl<LoadProfileMenu>
{
public:
	static const string title;

	LoadProfileMenu();
private:
	static vector<string> getProfiles();
	static vector<listAction> getLoadActions();

	static void loadProfile(string name);
	static void back();
};


class SceneSelect : public TextListMenuImpl<SceneSelect>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline SceneSelect() {}
protected:
	static void back();
};

class PauseMenu : public TextListMenuImpl<PauseMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline PauseMenu() {}
};

class GameOverMenu : public TextListMenuImpl<GameOverMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline GameOverMenu() {}
};

class ChamberCompletedMenu : public TextListMenuImpl<ChamberCompletedMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline ChamberCompletedMenu(PlayScene* playScene) : playScene(playScene) {}

	virtual bool init();
protected:
	string enemyStatsMsg();

	Label * text = nullptr;
	PlayScene* playScene;
};

class ReplayCompletedMenu : public TextListMenuImpl<ReplayCompletedMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline ReplayCompletedMenu() {}
};

class MapMenu : public Layer
{
public:
	static const int margin;
	static const Color4F backgroundColor;
	static const Color4F wallColor;
	static const Color4F floorColor;
	static const Color4F doorColor;

	MapMenu(PlayScene* playScene);

	virtual bool init();

	void close();
protected:
	unique_ptr<ControlListener> controlListener;
	PlayScene* playScene = nullptr;
	DrawNode* backgroundNode;
	DrawNode* drawNode;
	float _pixelsPerTile = 1.0f;

	void drawMaps();
	void drawObject(CCRect area, Color4F color);
};

#endif /* menu_layers_h */
