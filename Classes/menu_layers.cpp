//
//  menu_layers.cpp
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "Graphics.h"
#include "GSpace.hpp"
#include "menu_layers.h"
#include "menu_scenes.h"
#include "PlayScene.hpp"
#include "Wall.hpp"

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
	GScene::runScene("Mansion");
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
	"Sakuya Room",
	"Marisa Room",
	"Library",
	"FR1",
	"Desert",
	"Mine",
	"Ice Cave",
	"Back"
};

const vector<TextListMenuLayer::listAction> SceneSelect::entryActions = {
	{ sceneLaunchAdapterByName("Collect") },
	{ sceneLaunchAdapterByName("StalkerRoom") },
	{ sceneLaunchAdapterByName("Wander") },
	{ sceneLaunchAdapterByName("FacerFloor") },
	{ sceneLaunchAdapterByName("SakuyaRoom") },
	{ sceneLaunchAdapterByName("MarisaRoom") },
	{ sceneLaunchAdapterByName("Library") },
	{ sceneLaunchAdapterByName("FR1") },
	{ sceneLaunchAdapterByName("Desert") },
	{ sceneLaunchAdapterByName("Mine") },
	{ sceneLaunchAdapterByName("IceCave") },
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

const int MapMenu::margin = 64;

const Color4F MapMenu::backgroundColor(0.5f, 0.5f, 0.5f, 0.5f);
const Color4F MapMenu::wallColor(0.3f, 0.3f, 0.7f, 1.0f);

MapMenu::MapMenu(PlayScene* playScene) :
	playScene(playScene),
	controlListener(make_unique<ControlListener>())
{
}

bool MapMenu::init()
{
	Layer::init();

	controlListener->addPressListener(ControlAction::menuBack, bind(&MapMenu::close, this));

	Vec2 size = getScreenSize();
	SpaceVect areaSize = playScene->getMapSize();

	drawNode = DrawNode::create();
	addChild(drawNode);
	drawNode->drawSolidRect(
		Vec2(margin, margin),
		Vec2(size.x - margin, size.y - margin),
		backgroundColor
	);

	_pixelsPerTile = min((size.x-margin*2) / areaSize.x, (size.y-margin*2) / areaSize.y);

	drawMaps();

	return true;
}

void MapMenu::close()
{
	playScene->exitMap();
}

void MapMenu::drawMaps()
{
	vector<object_ref<Wall>> walls = playScene->getSpace()->getObjectsByTypeAs<Wall>();

	for (auto ref : walls)
	{
		Wall* wall = ref.get();

		CCRect rect = wall->getBoundingBox();

		drawNode->drawSolidRect(
			Vec2(rect.getMinX(), rect.getMinY()) * _pixelsPerTile + Vec2(margin,margin),
			Vec2(rect.getMaxX(), rect.getMaxY()) * _pixelsPerTile + Vec2(margin,margin),
			wallColor
		);
	}
}
