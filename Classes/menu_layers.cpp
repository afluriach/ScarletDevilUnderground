//
//  menu_layers.cpp
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#include "Prefix.h"

#include "App.h"
#include "Attributes.hpp"
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
zero_arity_function sceneLaunchAdapter() {
	return []() -> void { app->runScene<T>(); };
}

zero_arity_function sceneLaunchAdapterByName(const string& name) {
	return [=]() -> void { GScene::runScene(name); };
}

const string TitleMenu::title = App::title;

const vector<string> TitleMenu::entries = {
	"New Game",
	"Load Game",
	"Load Replay",
	"Scene Select",
	"World Select",
	"Exit"
};

const vector <zero_arity_function > TitleMenu::entryActions = {
	newGame,
	loadGame,
	loadReplay,
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

void TitleMenu::loadReplay()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<LoadReplayMenu>());
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
	TextListMenuLayer(title, getProfileEntries())
{}

vector<pair<string, zero_arity_function>> NewProfileMenu::getProfileEntries()
{
	vector<entry> result;

	for (int i = 1; i <= GState::maxProfiles; ++i)
	{
		string profileDesc = boost::str(
			boost::format("%d: %s") %
			i %
			(GState::profileSlotsInUse.at(i - 1) ? "In Use" : "Empty")
		);
		zero_arity_function f = bind(
			&NewProfileMenu::selectProfile,
			string(boost::str(boost::format("profile%d") % i))
		);
		result.push_back(make_pair(profileDesc, f));
	}

	return result;
}

void NewProfileMenu::selectProfile(string name)
{
	App::crntProfileName = name;
	log("Profile %s selected.", name.c_str());
	App::runOpeningScene();
}

vector<TextListMenuLayer::entry> FileSelectMenu::generateEntries(set<string> fileNames, function<void(string)> handler)
{
	vector<pair<string, zero_arity_function>> result;

	for (string s : fileNames) {
		result.push_back(make_pair(s, bind(handler, s)));
	}
	result.push_back(make_pair("Back", &App::popMenu));

	return result;
}

FileSelectMenu::FileSelectMenu(string title, set<string> fileNames, function<void(string)> handler) :
	TextListMenuLayer(title, generateEntries(fileNames, handler))
{
}

const string LoadProfileMenu::title = "Load Game";

LoadProfileMenu::LoadProfileMenu() :
	FileSelectMenu(title, io::getProfiles(), LoadProfileMenu::openProfile)
{

}

void LoadProfileMenu::openProfile(string name)
{
	App::getCrntScene()->pushMenu(Node::ccCreate<LoadProfileDetailMenu>(name));
}

LoadProfileDetailMenu::LoadProfileDetailMenu(string profileName) :
	profileName(profileName)
{

}

LoadProfileDetailMenu::~LoadProfileDetailMenu()
{
}

bool LoadProfileDetailMenu::init()
{
	MenuLayer::init();

	title = createTextLabel(profileName, 48);
	title->setPosition(App::width / 2, App::height - 64);
	addChild(title);

	profileState = io::loadProfileState(profileName);

	if (profileState) {
		attributes = make_unique<AttributeSystem>(profileState->getPlayerStats());

		info = Node::ccCreate<PlayerInfo>(attributes.get());
		info->setPosition(App::width * 0.75f, App::height * 0.66f);
		addChild(info, 3);

		Label* time = createTextLabel("Elapsed time: " + getTimeString(profileState->totalChamberTime()), 32);
		time->setPosition(App::width * 0.5f, App::height * 0.666f);
		addChild(time, 3);

		string levelsMsg = boost::str(
			boost::format("Levels cleared: %d / %d") % 
			profileState->chambersCompletedCount() % 
			to_int(ChamberID::end)
		);
		Label* levels = createTextLabel(levelsMsg, 32);
		levels->setPosition(App::width * 0.5f, App::height * 0.75f);
		addChild(levels, 3);
	}

	else {
		Label* err = createTextLabel("Error loading file.", 48);
		err->setPosition(App::width * 0.5f, App::height * 0.5f);
		addChild(err);
	}

	return true;
}

void LoadProfileDetailMenu::selectPressed()
{
	loadProfile();
}

void LoadProfileDetailMenu::backPressed()
{
	App::popMenu();
}

void LoadProfileDetailMenu::loadProfile()
{
	App::loadProfile(profileName);
	App::runOverworldScene();
}

const string LoadReplayMenu::title = "Load Replay";

LoadReplayMenu::LoadReplayMenu() :
	FileSelectMenu(title, io::getReplays(), LoadReplayMenu::loadReplay)
{
}

void LoadReplayMenu::loadReplay(string filename)
{
	GScene::runSceneWithReplay(filename);
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

const vector<zero_arity_function> SceneSelect::entryActions = {
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

	{ &App::popMenu }
};

zero_arity_function menuPushAdapter(string sceneName) {
	return [sceneName]() -> void {
		GScene* scene = App::getCrntScene();
		WorldSelect::nextScene = sceneName;
		scene->pushMenu(Node::ccCreate<CharacterSelect>());
	};
}

const string WorldSelect::title = "Select World";

#define entry(x) {#x, menuPushAdapter(#x)}

const vector<pair<string,zero_arity_function>> WorldSelect::entries = {
	entry(Graveyard1),
	entry(Graveyard2),
	entry(Graveyard3),
	entry(Graveyard4),
	entry(Forest),
	entry(Desert),
	entry(Mine),
	{ "Back", &WorldSelect::back }
};

#undef entry

const vector<ChamberID> WorldSelect::chamberIDs = {
	ChamberID::graveyard1,
	ChamberID::graveyard2,
	ChamberID::graveyard3,
	ChamberID::graveyard4,
	ChamberID::forest1,
	ChamberID::desert1,
	ChamberID::mine1,
};

string WorldSelect::nextScene = "";

vector<pair<string, zero_arity_function>> WorldSelect::getAvailableEntries()
{
	vector<pair<string,zero_arity_function>> result;
	GState* state = App::crntState.get();

	for_irange(i, 0, entries.size() - 1) {
		if (state->isChamberAvailable(chamberIDs.at(i))) {
			result.push_back(entries.at(i));
		}
	}
	result.push_back(entries.back());

	return result;
}

WorldSelect::WorldSelect(bool showAll) :
	TextListMenuLayer(
		"WorldSelect",
		showAll ? entries : getAvailableEntries()
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

zero_arity_function characterSelectAdapter(PlayerCharacter pc) {
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

const vector<zero_arity_function> CharacterSelect::entryActions = {
	characterSelectAdapter(PlayerCharacter::flandre),
	characterSelectAdapter(PlayerCharacter::rumia),
	characterSelectAdapter(PlayerCharacter::cirno),
	&App::popMenu
};

const string PauseMenu::title = "-PAUSED-";

const vector<string> PauseMenu::overworldEntries = {
	"Resume",
	"Dream World Select",
	"Save",
	"Exit to title"
};

const vector<zero_arity_function> PauseMenu::overworldEntryActions = {
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

const vector<zero_arity_function> PauseMenu::chamberEntryActions = {
	&App::resumeScene,
	&App::restartScene,
	static_cast<GScene*(*)(void)>(&App::runOverworldScene)
};

PauseMenu::PauseMenu(bool overworld, Player* player) :
	TextListMenuLayer(
		title,
		overworld ? overworldEntries : chamberEntries,
		overworld ? overworldEntryActions : chamberEntryActions
	),
	player(player)
{
	PlayerInfo* info = Node::ccCreate<PlayerInfo>(player->getAttributeSystem());
	info->setPosition(App::width * 0.75f, App::height * 0.66f);
	addChild(info, 3);
}

void PauseMenu::worldSelect()
{
	App::getCrntScene()->pushMenu(Node::ccCreate<WorldSelect>(false));
}

const string GameOverMenu::title = "GAME OVER";

const vector<string> GameOverMenu::entries = {
	"Restart",
	"Save Replay",
	"Exit to title"
};

const vector<zero_arity_function> GameOverMenu::entryActions = {
	&App::restartScene,
	[]()->void {
		if (auto playScene = dynamic_cast<PlayScene*>(App::getCrntScene()))
			playScene->autosaveReplayData();
	},
	&App::runTitleScene
};

const string ChamberCompletedMenu::title = "COMPLETED!";

const vector<pair<string,zero_arity_function>> ChamberCompletedMenu::entries = {
	{ "Return to Overworld", static_cast<GScene*(*)(void)>(&App::runOverworldScene) },
	{ "Play again", &App::restartScene },
	{ "Exit to title", &App::runTitleScene}
};

vector<TextListMenuLayer::entry> ChamberCompletedMenu::getEntries(ChamberID nextLevel)
{
	vector<entry> result;

	if (nextLevel != ChamberID::invalid_id) {
		result.push_back(make_pair(
			"Next Level",
			[nextLevel]() -> void { PlayScene::runScene(nextLevel); }
		));
	}

	for (auto entry : entries) result.push_back(entry);

	return result;
}

ChamberCompletedMenu::ChamberCompletedMenu(PlayScene* playScene) :
	TextListMenuImpl(getEntries(playScene->getNextLevel())),
	playScene(playScene)
{}

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

	if (playScene->getCurrentLevel() != ChamberID::invalid_id) {
		*App::crntState.get() = *playScene->getSpace()->getState();
		updateSaveState();

		ChamberID nextID = playScene->getNextLevel();
		if (nextID != ChamberID::invalid_id) {
			App::crntState->registerChamberAvailable(nextID);
		}
	}
	else {
		log("ChamberCompletedMenu: invalid ID");
	}

	App::autosaveProfile();
	playScene->autosaveReplayData();

	return true;
}

void ChamberCompletedMenu::updateSaveState()
{
	ChamberID crnt = playScene->getCurrentLevel();
	if (crnt == ChamberID::invalid_id) return;

	ChamberStats& crntStats = App::crntState->chamberStats.at(to_size_t(crnt));
	unsigned int timeMS = frameCount * App::secondsPerFrame * 1000;
	unsigned char enemies = totalEnemyCount().first;

	++crntStats.timesCompleted;
	crntStats.totalTimeMS += timeMS;

	if (crntStats.fastestTimeMS != 0 && timeMS < crntStats.fastestTimeMS) {
		crntStats.fastestTimeMS = timeMS;
	}

	crntStats.maxEnemiesDefeated = max(crntStats.maxEnemiesDefeated, enemies);
}

string ChamberCompletedMenu::statsMsg()
{
	unsigned int millis = frameCount * App::secondsPerFrame * 1000.0;
	pair<unsigned int, unsigned int> enemyTotals = totalEnemyCount();

	return boost::str(boost::format("Clear time: %s\nEnemies defeated: %d / %d") %
		getTimeString(millis) %
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

const vector<zero_arity_function> ReplayCompletedMenu::entryActions = {
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
	{ Attribute::agility, "Agility" },
	{ Attribute::attack, "Attack" },
	{ Attribute::bulletCount, "Bullet Count" },
	{ Attribute::maxHP, "Max HP" },
	{ Attribute::maxMP, "Max MP" },
	{ Attribute::maxStamina, "Max Stamina" },
	{ Attribute::ricochet, "Ricochet" },
	{ Attribute::shieldLevel, "Shield" },
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
		string str = boost::str(boost::format("%s: %s") % displayAttributes[i].second.c_str() % boost::lexical_cast<string>((*stats)[displayAttributes[i].first]));
		Label* label = createTextLabel(str, 32*scale);
		label->setPosition(0.0f, -64.0f * i * scale);
		label->setAlignment(TextHAlignment::RIGHT);
		addChild(label);

		Sprite* icon = Sprite::create(AttributeSystem::upgradeAttributes.at(displayAttributes[i].first).sprite);
		icon->setScale(0.25f * scale);
		icon->setPosition(-192.0f * scale, -64.0f * i * scale);
		addChild(icon);
	}

	return true;
}
