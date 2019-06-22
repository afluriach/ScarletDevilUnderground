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
#if DEV_MODE
	"Scene Select",
	"World Select",
#endif
	"Exit"
};

const vector <zero_arity_function > TitleMenu::entryActions = {
	newGame,
	loadGame,
	loadReplay,
#if DEV_MODE
	sceneSelect,
	worldSelect,
#endif
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

	unsigned int height = app::params.height;
	unsigned int width = app::params.width;

	title = createTextLabel(profileName, 48);
	title->setPosition(width / 2, height - 64);
	addChild(title);

	profileState = io::loadProfileState(profileName);

	if (profileState) {
		attributes = make_unique<AttributeSystem>(profileState->getPlayerStats());

		info = Node::ccCreate<PlayerInfo>(attributes.get());
		info->setPosition(width * 0.75f, height * 0.66f);
		addChild(info, 3);

		Label* time = createTextLabel("Elapsed time: " + getTimeString(profileState->totalChamberTime()), 32);
		time->setPosition(width * 0.5f, height * 0.666f);
		addChild(time, 3);

		string levelsMsg = boost::str(
			boost::format("Levels cleared: %d / %d") % 
			profileState->chambersCompletedCount() % 
			to_int(ChamberID::end)
		);
		Label* levels = createTextLabel(levelsMsg, 32);
		levels->setPosition(width * 0.5f, height * 0.75f);
		addChild(levels, 3);
	}

	else {
		Label* err = createTextLabel("Error loading file.", 48);
		err->setPosition(width * 0.5f, height * 0.5f);
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
		GScene::runScene(sceneName);
	};
}

const string WorldSelect::title = "Select World";

#define entry(x) {#x, menuPushAdapter(#x)}

const vector<pair<string,zero_arity_function>> WorldSelect::entries = {
	entry(Graveyard1),
	entry(Graveyard2),
	entry(Graveyard3),
	entry(Graveyard4),
#if DEV_MODE
	entry(Forest),
	entry(Desert),
	entry(Mine),
#endif
	{ "Back", &WorldSelect::back }
};

#undef entry

const vector<ChamberID> WorldSelect::chamberIDs = {
	ChamberID::graveyard1,
	ChamberID::graveyard2,
	ChamberID::graveyard3,
	ChamberID::graveyard4,
#if DEV_MODE
	ChamberID::forest1,
	ChamberID::desert1,
	ChamberID::mine1,
#endif
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

const string PauseMenu::title = "";

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

	info->setPosition(app::params.width * 0.75f, app::params.height * 0.66f);
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
	unsigned int height = app::params.height;
	unsigned int width = app::params.width;

	Label* enemyStatsLabel = createTextLabel(enemyStatsMsg(),24*scale, "fonts/coiny.ttf");
	addChild(enemyStatsLabel, 0);
	enemyStatsLabel->setPosition(Vec2(width * 0.75f, height * 0.5f));

	Label* statsLabel = createTextLabel(statsMsg(), 24*scale, "fonts/coiny.ttf");
	addChild(statsLabel, 0);
	statsLabel->setPosition(Vec2(width * 0.5f, height * 0.5f));

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
	unsigned int timeMS = frameCount * app::params.secondsPerFrame * 1000;
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
	unsigned int millis = frameCount * app::params.secondsPerFrame * 1000.0;
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
			GObject::properNameByType(entry.first) + " : " + 
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
const Color4F MapMenu::wallColorFade = hsva4F(0.0f, 0.0f, 0.3f, 1.0f);
const Color4F MapMenu::floorColor(0.45f, 0.45f, 0.85f, 1.0f);
const Color4F MapMenu::floorColorCrnt(0.6f, 0.6f, 0.9f, 1.0f);
const Color4F MapMenu::floorColorFade = hsva4F(0.0f, 0.0f, 0.6f, 1.0f);
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

	for (auto ref : floors)
	{
		FloorSegment* floor = ref.get();

		if (dynamic_cast<MovingPlatform*>(floor) || floor->isHidden()) {
			continue;
		}
		else if(dynamic_cast<Pitfall*>(floor)) {
			drawObject(floor->getBoundingBox(), Color4F::BLACK);
		}
		else {
			drawObject(floor->getBoundingBox(), floorColor, floorColorCrnt, floorColorFade);
		}
	}

	for (auto ref : walls){
		if (!ref.get()->isHidden()) {
			drawObject(ref.get()->getBoundingBox(), wallColor, wallColorCrnt, wallColorFade);
		}
	}

	for (auto ref : enemies) {
		if (!ref.get()->isHidden()) {
			drawObject(ref.get()->getBoundingBox(), enemyColor);
		}
	}

	for (auto ref : doors) {
		if (!ref.get()->isHidden()) {
			drawObject(
				ref.get()->getBoundingBox(),
				ref.get()->isLocked() ? Color4F::RED : doorColor
			);
		}
	}

	if (goal) {
		drawObject(goal->getBoundingBox(), goalColor);
	}

	if (player) {
		drawObject(player->getBoundingBox(), Color4F::GRAY, Color4F::WHITE, Color4F::GRAY);
	}
}

void MapMenu::drawObject(SpaceRect rect, Color4F color)
{
	drawObject(rect, color, color, color);
}

void MapMenu::drawObject(SpaceRect rect, Color4F color, Color4F highlightColor, Color4F fadeColor)
{
	int mapId = playScene->getMapLocation(rect);
	if (mapId == -1) return;

	Color4F _color;
	bool visitedRoom = playScene->getMapAreasVisited().at(mapId);
	bool discoveredRoom = playScene->getAreasVisibleOnMap().at(mapId);

	if (mapId == playerRoom && isHighlight) {
		_color = highlightColor;
	}
	else if (visitedRoom) {
		_color = color;
	}
	else if (discoveredRoom) {
		_color = fadeColor;
	}

	drawNode->drawSolidRect(
		Vec2(rect.getMinX(), rect.getMinY()) * _pixelsPerTile + Vec2(margin, margin),
		Vec2(rect.getMaxX(), rect.getMaxY()) * _pixelsPerTile + Vec2(margin, margin),
		_color
	);
}

const vector<PlayerInfo::IncidentAttrEntry> PlayerInfo::displayIncidentAttributes = {
	{ Attribute::hp, Attribute::maxHP, "HP" },
	{ Attribute::mp, Attribute::maxMP, "MP" },
	{ Attribute::stamina, Attribute::maxStamina, "Stamina" },
};

const vector<pair<Attribute, string>> PlayerInfo::displayAttributes = {
	{ Attribute::agility, "Agility" },
	{ Attribute::attack, "Attack Damage" },
	{ Attribute::attackSpeed, "Attack Speed" },
	{ Attribute::bulletSpeed, "Bullet Speed" },
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

	for_irange(i, 0, displayIncidentAttributes.size())
	{
		auto entry = displayIncidentAttributes.at(i);
		float attr = (*stats)[get<0>(entry)];
		float maxAttr = (*stats)[get<1>(entry)];
		string icon = AttributeSystem::upgradeAttributes.at(get<1>(entry)).sprite;
		string text = boost::str(boost::format("%s: %s / %s") %
			get<2>(entry) %
			floatToStringOptionalDecimal(attr) %
			//Should be an integer value.
			boost::lexical_cast<string>(maxAttr)
		);

		addEntry(icon, text, i);
	}

	for_irange(i, 0, displayAttributes.size())
	{
		auto entry = displayAttributes.at(i);
		string icon = AttributeSystem::upgradeAttributes.at(entry.first).sprite;
		string text = boost::str(boost::format("%s: %s") %
			entry.second.c_str() %
			boost::lexical_cast<string>((*stats)[entry.first])
		);

		addEntry(icon, text, i + displayIncidentAttributes.size());
	}

	return true;
}

void PlayerInfo::addEntry(string icon, string text, int position)
{
	float scale = App::getScale();

	Label* label = createTextLabel(text, 32 * scale);
	label->setPosition(0.0f, -64.0f * position * scale);
	label->setAlignment(TextHAlignment::RIGHT);
	addChild(label);

	Sprite* _icon = Sprite::create("sprites/" + icon + ".png");
	_icon->setScale(0.25f * scale);
	_icon->setPosition(-192.0f * scale, -64.0f * position * scale);
	addChild(_icon);
}
