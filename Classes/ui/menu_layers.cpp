//
//  menu_layers.cpp
//  Koumachika
//
//  Created by Toni on 12/16/18.
//
//

#include "Prefix.h"

#include "Door.hpp"
#include "Enemy.hpp"
#include "FileIO.hpp"
#include "FloorSegment.hpp"
#include "Graphics.h"
#include "Item.hpp"
#include "menu_layers.h"
#include "menu_scenes.h"
#include "PlayScene.hpp"
#include "Player.hpp"
#include "Wall.hpp"

zero_arity_function sceneLaunchAdapterByName(const string& name) {
	return [=]() -> void { App::runPlayScene(name, "player_start"); };
}

const string TitleMenu::title = App::title;

const vector<string> TitleMenu::entries = {
	"New Game",
	"Load Game",
#if DEV_MODE
	"Scene Select",
	"World Select",
#endif
	"Exit"
};

const vector <zero_arity_function > TitleMenu::entryActions = {
	newGame,
	loadGame,
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
	log1("Profile %s selected.", name.c_str());
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

		Label* time = createTextLabel("Elapsed time: " + getTimeString(profileState->getElapsedTime()), 32);
		time->setPosition(width * 0.5f, height * 0.666f);
		addChild(time, 3);

		string levelsMsg = boost::str(
			boost::format("Areas cleared: %d") % 
			profileState->areasClearedCount()
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
	App::runPlayScene(App::getCrntState()->area, App::getCrntState()->entrance);
}

const string SceneSelect::title = "Scene Select";

const vector<string> SceneSelect::entries = {
	"library2",
	"mansion_outside1",
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
	{ sceneLaunchAdapterByName("library2") },
	{ sceneLaunchAdapterByName("mansion_outside1") },
	{ sceneLaunchAdapterByName("Wander") },
	{ sceneLaunchAdapterByName("FacerFloor") },
	{ sceneLaunchAdapterByName("SakuyaRoom") },
	{ sceneLaunchAdapterByName("MarisaRoom") },
	{ sceneLaunchAdapterByName("library") },
	{ sceneLaunchAdapterByName("FR1") },
	{ sceneLaunchAdapterByName("ice_cave") },
	{ sceneLaunchAdapterByName("forest1") },

	{ &App::popMenu }
};

zero_arity_function menuPushAdapter(string sceneName) {
	return [sceneName]() -> void {
		App::runPlayScene(sceneName, "player_start");
	};
}

const string WorldSelect::title = "Select World";

#define entry(x) {#x, menuPushAdapter(#x)}

const vector<pair<string,zero_arity_function>> WorldSelect::entries = {
	entry(graveyard1),
	entry(graveyard2),
	entry(graveyard3),
	entry(graveyard4),
	entry(forest1),
	entry(desert1),
	entry(mine1),
	{ "Back", &WorldSelect::back }
};

#undef entry

const vector<string> WorldSelect::chamberIDs = {
	"graveyard1",
	"graveyard2",
	"graveyard3",
	"graveyard4",
	"forest1",
	"desert1",
	"mine1",
};

string WorldSelect::nextScene = "";

vector<pair<string, zero_arity_function>> WorldSelect::getAvailableEntries()
{
	vector<pair<string,zero_arity_function>> result;
	GState* state = App::crntState.get();

	for_irange(i, 0, entries.size() - 1) {
		if (state->getAttribute(chamberIDs.at(i))) {
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

const string PauseMenu::title = "";

const vector<string> PauseMenu::entries = {
	"Resume",
	"Save Game",
	"Exit to title"
};

const vector<zero_arity_function> PauseMenu::entryActions = {
	&App::resumeScene,
	&PauseMenu::saveMenu,
	&App::runTitleScene
};

PauseMenu::PauseMenu(bool overworld, Player* player) :
	TextListMenuLayer(
		title,
		entries,
		entryActions
	),
	player(player)
{
	InventoryInfo* inventory = Node::ccCreate<InventoryInfo>();
	inventory->setPosition(app::params.width * 0.25f, app::params.height * 0.5f);
	addChild(inventory, 3);

	if (player) {
		PlayerInfo* info = Node::ccCreate<PlayerInfo>(player->getAttributeSystem());

		info->setPosition(app::params.width * 0.75f, app::params.height * 0.66f);
		addChild(info, 3);
	}
}

void PauseMenu::saveMenu()
{
	App::saveCrntProfile();
}

const string GameOverMenu::title = "GAME OVER";

const vector<string> GameOverMenu::entries = {
	"Restart",
	"Exit to title"
};

const vector<zero_arity_function> GameOverMenu::entryActions = {
	&App::restartScene,
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

	playerRoom = playScene->getSpace()->getPlayerRoom();

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
	vector<gobject_ref> walls = playScene->getSpace()->getObjectsByTypeAs<Wall>();
	vector<gobject_ref> floors = playScene->getSpace()->getObjectsByTypeAs<FloorSegment>();
	vector<gobject_ref> pitfalls = playScene->getSpace()->getObjectsByTypeAs<Pitfall>();
	vector<gobject_ref> doors = playScene->getSpace()->getObjectsByTypeAs<Door>();
	vector<gobject_ref> enemies = playScene->getSpace()->getObjectsByTypeAs<Enemy>();
	GObject* goal = playScene->getSpace()->getObject("goal");
	GObject* player = playScene->getSpace()->getPlayer();

	for (auto ref : floors)
	{
		GObject* floor = ref.get();

		if (floor->isHidden()) {
			continue;
		}
		else {
			drawObject(floor->getBoundingBox(), floorColor, floorColorCrnt, floorColorFade);
		}
	}

	for (auto ref : pitfalls){
		GObject* floor = ref.get();
		drawObject(floor->getBoundingBox(), Color4F::BLACK);
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
				ref.getAs<Door>()->isLocked() ? Color4F::RED : doorColor
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
	{ Attribute::hp, Attribute::maxHP, "HP", "hp_upgrade" },
	{ Attribute::mp, Attribute::maxMP, "MP", "mp_upgrade" },
	{ Attribute::stamina, Attribute::maxStamina, "Stamina", "stamina_upgrade" },
};

const vector<PlayerInfo::AttrEntry> PlayerInfo::displayAttributes = {
	{ Attribute::agility, "Agility", "agility_upgrade" },
	{ Attribute::attack, "Attack Damage", "attack_upgrade" },
	{ Attribute::attackSpeed, "Attack Speed", "attack_speed_upgrade" },
	{ Attribute::bulletSpeed, "Bullet Speed", "bullet_speed_upgrade" },
	{ Attribute::shieldLevel, "Shield", "shield_upgrade" },
};

InventoryInfo::InventoryInfo()
{
}

bool InventoryInfo::init()
{
	Node::init();

	float scale = App::getScale();
	int idx = 0;

	for (auto const& entry : App::crntState->itemRegistry)
	{
		local_shared_ptr<item_properties> props = app::getItem(entry.first);
		if (entry.second == 0 || !props) continue;

		string text = props->properName;
		shared_ptr<sprite_properties> sprite = props->sprite;

		if (entry.second > 1) {
			text += " (" + boost::lexical_cast<string>(entry.second) + ")";
		}

		Label* label = createTextLabel(text, 32 * scale);
		label->setPosition(0.0f, -64.0f * idx * scale);
		label->setAlignment(TextHAlignment::RIGHT);
		addChild(label);

		if (sprite->filename.size() > 0) {
			Sprite* _icon = Sprite::create("sprites/" + sprite->filename + ".png");
			if (_icon) {
				_icon->setScale(0.25f * scale);
				_icon->setPosition(-192.0f * scale, -64.0f * idx * scale);
				addChild(_icon);
			}
		}

		++idx;
	}

	return true;
}

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
		string icon = get<3>(entry);
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
		string icon = get<2>(entry);
		string text = boost::str(boost::format("%s: %s") %
			get<1>(entry) %
			boost::lexical_cast<string>((*stats)[ get<0>(entry)])
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
