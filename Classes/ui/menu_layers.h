//
//  menu_layers.h
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#ifndef menu_layers_h
#define menu_layers_h

#include "menu.h"

class PlayerInfo;

class TitleMenu : public TextListMenuImpl<TitleMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<zero_arity_function> entryActions;

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
	static vector<pair<string,zero_arity_function>> getProfileEntries();

	static void selectProfile(string name);
};

class FileSelectMenu : public TextListMenuLayer
{
public:
	static vector<entry> generateEntries(set<string> fileNames, function<void(string)> handler);

	FileSelectMenu(string title, set<string> fileNames, function<void(string)> handler);
	inline virtual ~FileSelectMenu() {}
};

class LoadProfileMenu : public FileSelectMenu
{
public:
	static const string title;

	LoadProfileMenu();
	inline virtual ~LoadProfileMenu() {}
private:
	static void openProfile(string name);
};

class LoadProfileDetailMenu : public MenuLayer
{
public:
	LoadProfileDetailMenu(string profileName);
	virtual ~LoadProfileDetailMenu();

	virtual bool init();

	virtual void selectPressed();
	virtual void backPressed();
private:
	void loadProfile();

	unique_ptr<GState> profileState;
	string profileName;
	unique_ptr<AttributeSystem> attributes;
	
	Label* title;
	PlayerInfo* info;
};

class SceneSelect : public TextListMenuImpl<SceneSelect>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<zero_arity_function> entryActions;

	inline SceneSelect() {}
	inline virtual ~SceneSelect() {}
};

class WorldSelect : public TextListMenuLayer
{
public:
	static const string title;
	static const vector<string> chamberIDs;
	static const vector<pair<string,zero_arity_function>> entries;

	static string nextScene;

	static vector<pair<string,zero_arity_function>> getAvailableEntries();

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
	static const vector<zero_arity_function> entryActions;

	inline CharacterSelect() {}
	inline virtual ~CharacterSelect() {}
};

class PauseMenu : public TextListMenuLayer
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<zero_arity_function> entryActions;

	PauseMenu(bool overworld, Player* player);
	inline virtual ~PauseMenu() {}
protected:
	static void saveMenu();

	Player * player;
};

class GameOverMenu : public TextListMenuImpl<GameOverMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<zero_arity_function> entryActions;

	inline GameOverMenu() {}
	inline virtual ~GameOverMenu() {}
};

class MapMenu : public MenuLayer
{
public:
	static const int margin;
	static const Color4F backgroundColor;
	static const Color4F wallColor;
	static const Color4F wallColorCrnt;
	static const Color4F wallColorFade;
	static const Color4F floorColor;
	static const Color4F floorColorCrnt;
	static const Color4F floorColorFade;
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
	void drawObject(SpaceRect area, Color4F color);
	void drawObject(SpaceRect area, Color4F color, Color4F highlightColor, Color4F fadeColor);

};

class InventoryInfo : public MenuLayer
{
public:
	InventoryInfo();

	virtual bool init();
};

class PlayerInfo : public Node
{
public:
	typedef tuple<Attribute, Attribute, string, string> IncidentAttrEntry;
	typedef tuple<Attribute, string, string> AttrEntry;

	static const vector<IncidentAttrEntry> displayIncidentAttributes;
	static const vector<AttrEntry> displayAttributes;

	PlayerInfo(const AttributeSystem* stats);
	inline virtual ~PlayerInfo() {}

	virtual bool init();
protected:
	void addEntry(string icon, string text, int position);

	const AttributeSystem* stats;
};

#endif /* menu_layers_h */
