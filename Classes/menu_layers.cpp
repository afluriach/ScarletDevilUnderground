//
//  menu_layers.cpp
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "menu_layers.h"
#include "menu_scenes.h"

template <typename T>
TextListMenuLayer::listAction sceneLaunchAdapter() {
	return []() -> void { app->runScene<T>(); };
}

TextListMenuLayer::listAction sceneLaunchAdapterByName(const string& name) {
	return [=]() -> void { GScene::runScene(name); };
}

const string TitleMenu::title = App::title;

const vector<string> TitleMenu::entries = {
	"Start",
	"Scene Select",
	"Exit"
};

const vector <TextListMenuLayer::listAction > TitleMenu::entryActions = {
	start,
	sceneSelect,
	&App::end
};

void TitleMenu::start()
{
	GScene::runScene("BlockRoom");
}

void TitleMenu::sceneSelect()
{
	App::createAndRunScene<SceneSelectScene>();
}

const string SceneSelect::title = "Scene Select";

const vector<string> SceneSelect::entries = {
	"Collect",
	"Stalker Room",
	"Wander",
	"Facer Floor",
	"SakuyaRoom",
	"Library",
	"FR1",
	"Desert",
	"Mine",
	"Back"
};

const vector<TextListMenuLayer::listAction> SceneSelect::entryActions = {
	{ sceneLaunchAdapterByName("Collect") },
	{ sceneLaunchAdapterByName("StalkerRoom") },
	{ sceneLaunchAdapterByName("Wander") },
	{ sceneLaunchAdapterByName("FacerFloor") },
	{ sceneLaunchAdapterByName("SakuyaRoom") },
	{ sceneLaunchAdapterByName("Library") },
	{ sceneLaunchAdapterByName("FR1") },
	{ sceneLaunchAdapterByName("Desert") },
	{ sceneLaunchAdapterByName("Mine") },
	{ SceneSelect::back }
};


void SceneSelect::back()
{
	App::createAndRunScene<TitleMenuScene>();
}

const string PauseMenu::title = "-PAUSED-";

const vector<string> PauseMenu::entries = {
	"Resume",
	"Restart",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> PauseMenu::entryActions = {
	&App::resumeScene,
	&App::restartScene,
	&App::runTitleScene
};

const string GameOverMenu::title = "GAME OVER";

const vector<string> GameOverMenu::entries = {
	"Restart",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> GameOverMenu::entryActions = {
	&App::restartScene,
	&App::runTitleScene
};

const string ChamberCompletedMenu::title = "COMPLETED!";

const vector<string> ChamberCompletedMenu::entries = {
	"Retry",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> ChamberCompletedMenu::entryActions = {
	&App::restartScene,
	&App::runTitleScene
};

const string ReplayCompletedMenu::title = "End of replay";

const vector<string> ReplayCompletedMenu::entries = {
	"Replay",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> ReplayCompletedMenu::entryActions = {
	&GScene::restartReplayScene,
	&App::runTitleScene
};
