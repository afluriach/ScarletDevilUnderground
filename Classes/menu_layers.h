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

class TitleMenu : public TextListMenuImpl<TitleMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline TitleMenu() {}
private:
	static void start();
	static void sceneSelect();
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

	inline ChamberCompletedMenu() {}
};

class ReplayCompletedMenu : public TextListMenuImpl<ReplayCompletedMenu>
{
public:
	static const string title;
	static const vector<string> entries;
	static const vector<listAction> entryActions;

	inline ReplayCompletedMenu() {}
};

#endif /* menu_layers_h */
