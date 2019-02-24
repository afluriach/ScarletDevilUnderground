//
//  menu_layers.h
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#ifndef menu_layers_h
#define menu_Layers_h

#include "Attributes.hpp"
#include "menu.h"
#include "types.h"

class PlayScene;
class Player;

class TitleMenu : public TextListMenuImpl<TitleMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline TitleMenu() {}
	inline virtual ~TitleMenu() {}
private:
	static void newGame();
	static void loadGame();
	static void sceneSelect();
	static void worldSelect();
};

class NewProfileMenu : public TextListMenuLayer
{
public:
	static const string title;

	NewProfileMenu();
	inline virtual ~NewProfileMenu() {}
private:
	static vector<string> getProfileSlots();
	static vector<listAction> getSelectionActions();

	static void selectProfile(string name);

	static void back();
};

class LoadProfileMenu : public TextListMenuImpl<LoadProfileMenu>
{
public:
	static const string title;

	LoadProfileMenu();
	inline virtual ~LoadProfileMenu() {}
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
	inline virtual ~SceneSelect() {}
protected:
	static void back();
};

class WorldSelect : public TextListMenuLayer
{
public:
	static const string title;
	static const vector<ChamberID> chamberIDs;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	static string nextScene;

	static vector<string> getAvailableChambers();
	static vector<listAction> getAvailableChamberActions();

	WorldSelect(bool showAll);
	inline virtual ~WorldSelect() {}

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
	inline virtual ~CharacterSelect() {}
protected:
	static void back();
};

class PauseMenu : public TextListMenuLayer
{
public:
	static const string title;
	static const vector<string> overworldEntries;
	static const vector<listAction> overworldEntryActions;
	static const vector<string> chamberEntries;
	static const vector<listAction> chamberEntryActions;

	static void worldSelect();

	PauseMenu(bool overworld, Player* player);
	inline virtual ~PauseMenu() {}
protected:
	Player * player;
};

class GameOverMenu : public TextListMenuImpl<GameOverMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline GameOverMenu() {}
	inline virtual ~GameOverMenu() {}
};

class ChamberCompletedMenu : public TextListMenuImpl<ChamberCompletedMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline ChamberCompletedMenu(PlayScene* playScene) : playScene(playScene) {}
	inline virtual ~ChamberCompletedMenu() {}

	virtual bool init();
protected:
	string statsMsg();
	string enemyStatsMsg();
	pair<unsigned int, unsigned int> totalEnemyCount();

	unsigned int frameCount;
	EnemyStatsMap enemyStats;
	PlayScene* playScene;
};

class ReplayCompletedMenu : public TextListMenuImpl<ReplayCompletedMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline ReplayCompletedMenu() {}
	inline virtual ~ReplayCompletedMenu() {}
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
	static const Color4F enemyColor;

	MapMenu(PlayScene* playScene);
	inline virtual ~MapMenu() {}

	virtual bool init();
	virtual void update(float dt);

	void backPressed();
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

class PlayerInfo : public Node
{
public:
	static const vector<pair<Attribute, string>> displayAttributes;

	PlayerInfo(const AttributeSystem* stats);
	inline virtual ~PlayerInfo() {}

	virtual bool init();
protected:
	const AttributeSystem* stats;
};

#endif /* menu_layers_h */
