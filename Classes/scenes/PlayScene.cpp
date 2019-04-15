//
//  PlayScene.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "controls.h"
#include "Dialog.hpp"
#include "FileIO.hpp"
#include "functional.hpp"
#include "GObject.hpp"
#include "GScene.hpp"
#include "GSpace.hpp"
#include "HUD.hpp"
#include "macros.h"
#include "menu_layers.h"
#include "multifunction.h"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "replay.h"
#include "util.h"
#include "value_map.hpp"

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    for(const Value& obj: objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        printValueMap(objAsMap);
    }
}

const Color3B PlayScene::fadeoutColor = Color3B(192,96,96);
const float PlayScene::fadeoutLength = 3.0f;

PlayScene* PlayScene::runScene(ChamberID id)
{
	auto it = adapters.find(id);

	if (it == adapters.end()) {
		log("runScene: invalid chamber ID %d", id);
		return nullptr;
	}
	else
	{
		return it->second();
	}
}

PlayScene::PlayScene(const string& mapName) :
	PlayScene(mapName, singleMapEntry(mapName))
{}

PlayScene::PlayScene(const string& sceneName, const vector<MapEntry>& maps) :
GScene(sceneName, maps)
{
    multiInit.insertWithOrder(
        wrap_method(PlayScene,addHUD,this),
        to_int(initOrder::initHUD)
    );

    control_listener->addPressListener(
        ControlAction::pause,
        [=]()-> void {onPausePressed(); }
    );

	control_listener->addPressListener(
		ControlAction::map_menu,
		[=]()-> void {onMapPressed(); }
	);

	App::resumeSounds();
}

PlayScene::~PlayScene()
{
}

void PlayScene::update(float dt)
{
	GScene::update(dt);

	if(!isPaused) hud->update();
}

void PlayScene::applyCameraControls()
{
    Vec2 arrowState = toCocos(App::control_register->getRightVector());
    if(!arrowState.isZero()){
		Vec2 cameraPos = Vec2(cameraArea.center.x, cameraArea.center.y);

		setUnitPosition(toChipmunk(cameraPos + arrowState * cameraMoveTilesPerSecond * App::secondsPerFrame));
    }
}

void PlayScene::addHUD()
{
    hud = Node::ccCreate<HUD>(gspace);
    getLayer(sceneLayers::hud)->addChild(hud);
	hud->setPerformanceStats();

	int mapCount = getMapFragmentsList().size();
	if (mapCount > 0) hud->initMapCounter(mapCount);
}

void PlayScene::onPausePressed()
{
	if (!isPaused) {
		enterPause();
	}
	else if (isPaused && menuStack.size() == 1) {
		exitPause();
	}
}

void PlayScene::onMapPressed()
{
	if (mapMenu)
		exitMap();
	else
		enterMap();
}


void PlayScene::enterPause()
{
	if (isShowingMenu)
		return;

    pauseAnimations();
	App::pauseSounds();
	setPaused(true);
	isShowingMenu = true;
	hud->showHidden();

	waitForSpaceThread();

	pauseMenu = Node::ccCreate<PauseMenu>(isOverworld, gspace->getObjectAs<Player>("player"));
	pushMenu(pauseMenu);
}

void PlayScene::exitPause()
{
	popMenu();
	pauseMenu = nullptr;
    resumeAnimations();
	App::resumeSounds();
	setPaused(false);
	isShowingMenu = false;
	hud->resetAutohide();
}

void PlayScene::pauseAnimations()
{
    getSpaceLayer()->pauseRecursive();
    getLayer(sceneLayers::hud)->pauseRecursive();
}

void PlayScene::resumeAnimations()
{
    getSpaceLayer()->resumeRecursive();
    getLayer(sceneLayers::hud)->resumeRecursive();
}

void PlayScene::showGameOverMenu()
{
	setPaused(true);

	waitForSpaceThread();
	logPerformance();

	showMenu(Node::ccCreate<GameOverMenu>());
}

void PlayScene::triggerGameOver()
{
	setPaused(true);

	setColorFilter(toColor4F(fadeoutColor));

	triggerMenu(&PlayScene::showGameOverMenu);
}

void PlayScene::showSceneCompletedMenu()
{
	waitForSpaceThread();

	logPerformance();
	showMenu(Node::ccCreate<ChamberCompletedMenu>(this));
}

void PlayScene::triggerSceneCompleted()
{
	setPaused(true);
	triggerMenu(&PlayScene::showSceneCompletedMenu);
}

void PlayScene::showReplayCompletedMenu()
{
	logPerformance();
	showMenu(Node::ccCreate<ReplayCompletedMenu>());
}

void PlayScene::triggerReplayCompleted()
{
	setPaused(true);
	triggerMenu(&PlayScene::showReplayCompletedMenu);
}

void PlayScene::enterMap()
{
	if (isShowingMenu)
		return;

	pauseAnimations();
	App::pauseSounds();
	setPaused(true);
	isShowingMenu = true;

	waitForSpaceThread();

	mapMenu = Node::ccCreate<MapMenu>(this);
	pushMenu(mapMenu);
}

void PlayScene::exitMap()
{
	popMenu();
	mapMenu = nullptr;
	resumeAnimations();
	App::resumeSounds();
	setPaused(false);
	isShowingMenu = false;
}

void PlayScene::enterWorldSelect()
{
	if (isShowingMenu)
		return;

	pauseAnimations();
	App::pauseSounds();
	setPaused(true);
	isShowingMenu = true;

	waitForSpaceThread();

	pushMenu(Node::ccCreate<WorldSelect>(false));
}

void PlayScene::exitWorldSelect()
{
	popMenu();
	resumeAnimations();
	App::resumeSounds();
	setPaused(false);
	isShowingMenu = false;
}

GScene* PlayScene::getReplacementScene()
{
	return Node::ccCreate<PlayScene>(sceneName, maps);
}

bool PlayScene::loadReplayData(const string& filename)
{
	return loadReplayData(io::getControlReplay(filename));
}

bool PlayScene::loadReplayData(unique_ptr<Replay> _replay)
{
	bool b = _replay.get();

	gspace->loadReplay(move(_replay));

	isRunningReplay = b;
	return b;
}

void PlayScene::saveReplayData(const string& filename)
{
	if (!isRunningReplay) {
		io::saveControlReplay(filename, gspace->getReplay());
	}
}

void PlayScene::autosaveReplayData()
{
	if (!isRunningReplay) {
		io::autosaveControlReplay(sceneName, gspace->getReplay());
	}
}

void PlayScene::showMenu(MenuLayer* menu)
{
	hud->setVisible(false);
	if (dialog)
		dialog->setVisible(false);

	pushMenu(menu);
}

void PlayScene::triggerMenu(void (PlayScene::*m)(void))
{
	if (isShowingMenu)
		return;

	isShowingMenu = true;

	Director::getInstance()->getScheduler()->schedule(
		bind(m, this),
		this,
		0.0f,
		0,
		fadeoutLength,
		false,
		"showMenu"
	);
}
