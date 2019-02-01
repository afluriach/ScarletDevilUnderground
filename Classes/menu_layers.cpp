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
#include "Door.hpp"
#include "FileIO.hpp"
#include "FloorSegment.hpp"
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
	"New Game",
	"Load Game",
	"Scene Select",
	"Exit"
};

const vector <TextListMenuLayer::listAction > TitleMenu::entryActions = {
	&App::runOverworldScene,
	loadGame,
	sceneSelect,
	&App::end
};

void TitleMenu::loadGame()
{
	App::createAndRunScene<LoadProfileScene>();
}

void TitleMenu::sceneSelect()
{
	App::createAndRunScene<SceneSelectScene>();
}

const string LoadProfileMenu::title = "Load Game";

LoadProfileMenu::LoadProfileMenu() :
	TextListMenuImpl<LoadProfileMenu>(getProfiles(), getLoadActions())
{

}

vector<string> LoadProfileMenu::getProfiles()
{
	vector<string> result;

	auto profiles = io::getProfiles();

	for (string s : profiles) {
		result.push_back(s);
	}
	result.push_back("Back");

	return result;
}

vector<TextListMenuLayer::listAction> LoadProfileMenu::getLoadActions()
{
	vector<listAction> result;

	auto profiles = io::getProfiles();

	for (string s : profiles) {
		result.push_back(bind(&LoadProfileMenu::loadProfile, s));
	}
	result.push_back(&LoadProfileMenu::back);

	return result;
}

void LoadProfileMenu::loadProfile(string name)
{
	App::loadProfile(name);
	App::runOverworldScene();
}

void LoadProfileMenu::back()
{
	App::createAndRunScene<TitleMenuScene>();
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
	"Forest",
	"Graveyard",
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
	{ sceneLaunchAdapterByName("Forest") },
	{ sceneLaunchAdapterByName("Graveyard") },

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
	"Continue",
	"Retry",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> ChamberCompletedMenu::entryActions = {
	&App::runOverworldScene,
	&App::restartScene,
	&App::runTitleScene
};

bool ChamberCompletedMenu::init()
{
	TextListMenuLayer::init();

	text = createTextLabel(enemyStatsMsg(),24, "fonts/coiny.ttf");
	addChild(text, 0);
	text->setPosition(Vec2(App::width / 2.0f, App::height / 2.0f));

	return true;
}

string ChamberCompletedMenu::enemyStatsMsg()
{
	map<type_index, pair<unsigned int, unsigned int>> enemyStats = playScene->getSpace()->getEnemyStats();
	string result;

	result += "Enemies defeated:";

	for (auto entry : enemyStats)
	{
		result +=
			"\n" +
			string(entry.first.name()) + " : " + 
			boost::lexical_cast<string>(entry.second.first)  + " / " +
			boost::lexical_cast<string>(entry.second.second)
		;
	}

	return result;
}

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
const Color4F MapMenu::wallColorCrnt(0.45f, 0.45f, 0.85f, 1.0f);
const Color4F MapMenu::floorColor(0.45f, 0.45f, 0.85f, 1.0f);
const Color4F MapMenu::floorColorCrnt(0.6f, 0.6f, 0.9f, 1.0f);
const Color4F MapMenu::doorColor(0.61f,0.55f,0.39f,1.0f);

MapMenu::MapMenu(PlayScene* playScene) :
	playScene(playScene),
	controlListener(make_unique<ControlListener>())
{
}

bool MapMenu::init()
{
	Layer::init();

	controlListener->addPressListener(ControlAction::menuBack, bind(&MapMenu::close, this));
	scheduleUpdateWithPriority(0);

	Vec2 size = getScreenSize();
	SpaceVect areaSize = playScene->getMapSize();
	size_t roomCount = playScene->getMapAreas().size();

	playerRoom = playScene->getPlayerRoom();

	backgroundNode = DrawNode::create();
	addChild(backgroundNode,1);
	drawNode = DrawNode::create();
	addChild(drawNode, 2);

	backgroundNode->drawSolidRect(
		Vec2(margin, margin),
		Vec2(size.x - margin, size.y - margin),
		backgroundColor
	);

	_pixelsPerTile = min((size.x-margin*2) / areaSize.x, (size.y-margin*2) / areaSize.y);

	drawMaps();

	return true;
}

void MapMenu::update(float dt)
{
	timerDecrement(highlightTimer);

	if (highlightTimer == 0.0) {
		isHighlight = !isHighlight;
		highlightTimer = 1.0;

		drawNode->clear();
		drawMaps();
	}
}

void MapMenu::close()
{
	playScene->exitMap();
}

void MapMenu::drawMaps()
{
	vector<object_ref<Wall>> walls = playScene->getSpace()->getObjectsByTypeAs<Wall>();
	vector<object_ref<FloorSegment>> floors = playScene->getSpace()->getObjectsByTypeAs<FloorSegment>();
	vector<object_ref<Door>> doors = playScene->getSpace()->getObjectsByTypeAs<Door>();

	const vector<SpaceRect>& mapAreas = playScene->getMapAreas();
	const vector<bool>& mapAreasVisited = playScene->getMapAreasVisited();

	for (auto ref : floors)
	{
		FloorSegment* floor = ref.get();

		if (dynamic_cast<MovingPlatform*>(floor) || floor->hidden) {
			continue;
		}
		else if(dynamic_cast<Pitfall*>(floor)) {
			drawObject(floor->getBoundingBox(), Color4F::BLACK, Color4F::BLACK);
		}
		else {
			drawObject(floor->getBoundingBox(), floorColor, floorColorCrnt);
		}
	}

	for (auto ref : walls){
		if (!ref.get()->hidden) {
			drawObject(ref.get()->getBoundingBox(), wallColor, wallColorCrnt);
		}
	}

	for (auto ref : doors) {
		drawObject(ref.get()->getBoundingBox(), doorColor, doorColor);
	}
}

void MapMenu::drawObject(SpaceRect rect, Color4F color, Color4F colorCrnt)
{
	int mapId = playScene->getMapLocation(rect);

	if (mapId != -1 && playScene->getMapAreasVisited().at(mapId)) {
		drawNode->drawSolidRect(
			Vec2(rect.getMinX(), rect.getMinY()) * _pixelsPerTile + Vec2(margin, margin),
			Vec2(rect.getMaxX(), rect.getMaxY()) * _pixelsPerTile + Vec2(margin, margin),
			mapId == playerRoom && isHighlight ? colorCrnt : color
		);
	}
}

