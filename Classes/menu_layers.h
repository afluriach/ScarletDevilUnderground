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
#include "types.h"

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
	static void worldSelect();
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

class WorldSelect : public TextListMenuImpl<WorldSelect>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	static string nextScene;

	inline WorldSelect() {}

protected:
	static void back();
};

class CharacterSelect : public TextListMenuImpl<CharacterSelect>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline CharacterSelect() {}
protected:
	static void back();
};

class PauseMenu : public TextListMenuImpl<PauseMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	static void worldSelect();

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

class MapMenu : public MenuLayer
{
public:
	static const int margin;
	static const Color4F backgroundColor;
	static const Color4F wallColor;
	static const Color4F wallColorCrnt;
	static const Color4F floorColor;
	static const Color4F floorColorCrnt;
	static const Color4F doorColor;
	static const Color4F goalColor;

	MapMenu(PlayScene* playScene);

	virtual bool init();
	virtual void update(float dt);

	void close();
protected:
	PlayScene* playScene = nullptr;
	DrawNode* backgroundNode;
	DrawNode* drawNode;
	float _pixelsPerTile = 1.0f;

	int playerRoom;
	float highlightTimer = 1.0f;
	bool isHighlight = false;

	void drawMaps();
	void drawObject(SpaceRect area, Color4F color, Color4F colorCrnt);
};

#endif /* menu_layers_h */
