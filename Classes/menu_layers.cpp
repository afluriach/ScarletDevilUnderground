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
#include "Enemy.hpp"
#include "FileIO.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "GSpace.hpp"
#include "GState.hpp"
#include "menu_layers.h"
#include "menu_scenes.h"
#include "PlayScene.hpp"
#include "Player.hpp"
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
	"World Select",
	"Exit"
};

const vector <TextListMenuLayer::listAction > TitleMenu::entryActions = {
	newGame,
	loadGame,
	sceneSelect,
	worldSelect,
	&App::end
};

void TitleMenu::newGame()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<NewProfileMenu>());
}

void TitleMenu::loadGame()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<LoadProfileMenu>());
}

void TitleMenu::sceneSelect()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<SceneSelect>());
}

void TitleMenu::worldSelect()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<WorldSelect>(true));
}

const string NewProfileMenu::title = "New Game Profile";

NewProfileMenu::NewProfileMenu() :
	TextListMenuLayer(title, getProfileSlots(), getSelectionActions())
{}

vector<string> NewProfileMenu::getProfileSlots()
{
	vector<string> result;

	for (int i = 1; i <= GState::maxProfiles; ++i)
	{
		result.push_back(boost::str(
			boost::format("%d: %s") %
			i %
			(GState::profileSlotsInUse.at(i - 1) ? "In Use" : "Empty")
		));
	}

	return result;
}

vector<TextListMenuLayer::listAction> NewProfileMenu::getSelectionActions()
{
	vector<listAction> result;

	for (int i = 1; i <= GState::maxProfiles; ++i)
	{
		result.push_back(bind(
			&NewProfileMenu::selectProfile,
			string(boost::str(boost::format("profile%d") % i))
		));
	}

	return result;
}

void NewProfileMenu::selectProfile(string name)
{
	App::crntProfileName = name;
	log("Profile %s selected.", name.c_str());
	App::runOpeningScene();
}

void NewProfileMenu::back()
{
	App::getCrntScene()->popMenu();
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
	"Ice Cave",
	"Forest",
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
	{ sceneLaunchAdapterByName("IceCave") },
	{ sceneLaunchAdapterByName("Forest") },

	{ SceneSelect::back }
};


void SceneSelect::back()
{
	App::createAndRunScene<TitleMenuScene>();
}

TextListMenuLayer::listAction menuPushAdapter(string sceneName) {
	return [sceneName]() -> void {
		GScene* scene = App::getCrntScene();
		WorldSelect::nextScene = sceneName;
		scene->pushMenu(Node::ccCreate<CharacterSelect>());
	};
}

const string WorldSelect::title = "Select World";

const vector<string> WorldSelect::entries = {
	"Graveyard1",
	"Graveyard2",
	"Forest",
	"Desert",
	"Mine",
	"Back"
};

const vector<TextListMenuLayer::listAction> WorldSelect::entryActions = {
	menuPushAdapter("Graveyard1"),
	menuPushAdapter("Graveyard2"),
	menuPushAdapter("Forest"),
	menuPushAdapter("Desert"),
	menuPushAdapter("Mine"),
	&WorldSelect::back
};

const vector<ChamberID> WorldSelect::chamberIDs = {
	ChamberID::graveyard1,
	ChamberID::graveyard2,
	ChamberID::forest1,
	ChamberID::desert1,
	ChamberID::mine1,
};

string WorldSelect::nextScene = "";

vector<string> WorldSelect::getAvailableChambers()
{
	vector<string> result;
	GState* state = App::crntState.get();

	for_irange(i, 0, entries.size() - 1) {
		if (state->isChamberAvailable(chamberIDs.at(i))) {
			result.push_back(entries.at(i));
		}
	}
	result.push_back(entries.back());

	return result;
}

vector<TextListMenuLayer::listAction> WorldSelect::getAvailableChamberActions()
{
	vector<listAction> result;
	GState* state = App::crntState.get();

	for_irange(i, 0, entryActions.size() - 1) {
		if (state->isChamberAvailable(chamberIDs.at(i))) {
			result.push_back(entryActions.at(i));
		}
	}
	result.push_back(entryActions.back());

	return result;
}

WorldSelect::WorldSelect(bool showAll) :
	TextListMenuLayer(
		"WorldSelect",
		showAll ? entries : getAvailableChambers(),
		showAll ? entryActions : getAvailableChamberActions()
	)
{}

void WorldSelect::back()
{
	GScene* scene = App::getCrntScene();
	PlayScene* ps = dynamic_cast<PlayScene*>(scene);

	if (ps) {
		ps->exitWorldSelect();
	}
	else {
		scene->popMenu();
	}
}

TextListMenuLayer::listAction characterSelectAdapter(PlayerCharacter pc) {
	return [pc]() -> void {
		App::crntPC = pc;
		GScene::runScene(WorldSelect::nextScene);
	};
}

const string CharacterSelect::title = "Select Character";

const vector<string> CharacterSelect::entries = {
	"Flandre",
	"Rumia",
	"Cirno",
	"Back"
};

const vector<TextListMenuLayer::listAction> CharacterSelect::entryActions = {
	characterSelectAdapter(PlayerCharacter::flandre),
	characterSelectAdapter(PlayerCharacter::rumia),
	characterSelectAdapter(PlayerCharacter::cirno),
	&CharacterSelect::back
};

void CharacterSelect::back()
{
	GScene* scene = App::getCrntScene();
	scene->popMenu();
}

const string PauseMenu::title = "-PAUSED-";

const vector<string> PauseMenu::overworldEntries = {
	"Resume",
	"Dream World Select",
	"Save",
	"Exit to title"
};

const vector<TextListMenuLayer::listAction> PauseMenu::overworldEntryActions = {
	&App::resumeScene,
	&PauseMenu::worldSelect,
	&App::saveCrntProfile,
	&App::runTitleScene
};

const vector<string> PauseMenu::chamberEntries = {
	"Resume",
	"Restart",
	"Exit to Overworld"
};

const vector<TextListMenuLayer::listAction> PauseMenu::chamberEntryActions = {
	&App::resumeScene,
	&App::restartScene,
	&App::runOverworldScene
};

PauseMenu::PauseMenu(bool overworld, Player* player) :
	TextListMenuLayer(
		title,
		overworld ? overworldEntries : chamberEntries,
		overworld ? overworldEntryActions : chamberEntryActions
	),
	player(player)
{
	PlayerInfo* info = Node::ccCreate<PlayerInfo>(&player->getAttributeSystem());
	info->setPosition(App::width * 0.75f, App::height * 0.5f);
	addChild(info, 3);
}

void PauseMenu::worldSelect()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<WorldSelect>(false));
}

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

	enemyStats = playScene->getSpace()->getEnemyStats();
	frameCount = playScene->getSpace()->getFrame();
	float scale = App::getScale();

	Label* enemyStatsLabel = createTextLabel(enemyStatsMsg(),24*scale, "fonts/coiny.ttf");
	addChild(enemyStatsLabel, 0);
	enemyStatsLabel->setPosition(Vec2(App::width * 0.75f, App::height * 0.5f));

	Label* statsLabel = createTextLabel(statsMsg(), 24*scale, "fonts/coiny.ttf");
	addChild(statsLabel, 0);
	statsLabel->setPosition(Vec2(App::width * 0.5f, App::height * 0.5f));

	return true;
}

string ChamberCompletedMenu::statsMsg()
{
	int seconds = frameCount / App::framesPerSecond;
	int minutes = seconds / 60;
	seconds %= 60;
	int hundredth = frameCount - App::framesPerSecond*(seconds - minutes*60);

	pair<unsigned int, unsigned int> enemyTotals = totalEnemyCount();

	return boost::str(boost::format("Clear time: %02d:%02d.%02d\nEnemies defeated: %d / %d") %
		minutes %
		seconds %
		hundredth %
		enemyTotals.first %
		enemyTotals.second
	);
}

string ChamberCompletedMenu::enemyStatsMsg()
{
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

pair<unsigned int, unsigned int> ChamberCompletedMenu::totalEnemyCount()
{
	pair<unsigned int, unsigned int> result = make_pair(0u,0u);

	for (auto entry : enemyStats) {
		result.first += entry.second.first;
		result.second += entry.second.second;
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
const Color4F MapMenu::goalColor(0.73f, 0.77f, 0.45f, 1.0f);
const Color4F MapMenu::enemyColor(1.0f, 0.0f, 0.0f, 1.0f);

MapMenu::MapMenu(PlayScene* playScene) :
	playScene(playScene)
{
}

bool MapMenu::init()
{
	MenuLayer::init();

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

void MapMenu::backPressed()
{
	playScene->exitMap();
}

void MapMenu::drawMaps()
{
	vector<object_ref<Wall>> walls = playScene->getSpace()->getObjectsByTypeAs<Wall>();
	vector<object_ref<FloorSegment>> floors = playScene->getSpace()->getObjectsByTypeAs<FloorSegment>();
	vector<object_ref<Door>> doors = playScene->getSpace()->getObjectsByTypeAs<Door>();
	vector<object_ref<Enemy>> enemies = playScene->getSpace()->getObjectsByTypeAs<Enemy>();
	GObject* goal = playScene->getSpace()->getObject("goal");
	GObject* player = playScene->getSpace()->getObject("player");

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

	for (auto ref : enemies) {
		if (!ref.get()->hidden) {
			drawObject(ref.get()->getBoundingBox(), enemyColor, enemyColor);
		}
	}

	for (auto ref : doors) {
		if (!ref.get()->hidden) {
			drawObject(
				ref.get()->getBoundingBox(),
				ref.get()->isLocked() ? Color4F::RED : doorColor,
				ref.get()->isLocked() ? Color4F::RED : doorColor
			);
		}
	}

	if (goal) {
		drawObject(goal->getBoundingBox(), goalColor, goalColor);
	}

	if (player) {
		drawObject(player->getBoundingBox(), Color4F::GRAY, Color4F::WHITE);
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

const vector<pair<Attribute, string>> PlayerInfo::displayAttributes = {
	{Attribute::maxHP, "HP"},
	{Attribute::maxMP, "MP"},
	{Attribute::maxPower, "Power"},
	{Attribute::agility, "Agility"},
};

PlayerInfo::PlayerInfo(const AttributeSystem* stats) :
	stats(stats)
{
}

bool PlayerInfo::init()
{
	Node::init();

	//base and upgrade values - only the player can have them - and the bases are constant data.
	//get playscene/crntplayer and get base stats for that player.
	float scale = App::getScale();

	for_irange(i, 0, displayAttributes.size())
	{
		string str = boost::str(boost::format("%s: %.1f") % displayAttributes[i].second.c_str() % stats->getAdjustedValue(displayAttributes[i].first));
		Label* label = createTextLabel(str, 32*scale);
		label->setPosition(0.0f, -64.0f * i * scale);
		label->setAlignment(TextHAlignment::RIGHT);
		addChild(label);
	}

	return true;
}
