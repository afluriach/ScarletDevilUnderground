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
#include "Graphics.h"
#include "GSpace.hpp"
#include "macros.h"
#include "menu_layers.h"
#include "multifunction.h"
#include "Player.hpp"
#include "PlayScene.hpp"
#include "scenes.h"
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
	multiInit.insertWithOrder(
		wrap_method(PlayScene, initReplayData, this),
		to_int(initOrder::core)
	);

	multiUpdate.insertWithOrder(
		wrap_method(PlayScene, updateReplayData, this),
		to_int(updateOrder::updateControls)
	);

	multiUpdate.insertWithOrder(
		bind(&GScene::runActionsWithOrder, this, updateOrder::moveCamera),
		to_int(updateOrder::moveCamera)
	);

    control_listener->addPressListener(
        ControlAction::pause,
        [=]()-> void {onPausePressed(); }
    );

	control_listener->addPressListener(
		ControlAction::mapMenu,
		[=]()-> void {onMapPressed(); }
	);

	App::resumeSounds();
}

PlayScene::~PlayScene()
{
}

void PlayScene::applyCameraControls()
{
    Vec2 arrowState = toCocos(App::control_register->getRightVector());
    if(!arrowState.isZero()){
		Vec2 cameraPos = Vec2(cameraArea.center.x, cameraArea.center.y);

		setUnitPosition(toChipmunk(cameraPos + arrowState * cameraMoveTilesPerSecond * App::secondsPerFrame));
    }
}

//Rather than making an updater by capturing hud, just wrap it in a method to access hud from the supplied this.
void PlayScene::updateHUD()
{
    hud->update();
}

void PlayScene::addHUD()
{
    hud = Node::ccCreate<HUD>(gspace);
    getLayer(sceneLayers::hud)->addChild(hud);
    multiUpdate.insertWithOrder(
        wrap_method(PlayScene,updateHUD,this),
        to_int(updateOrder::hudUpdate)
    );
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

	while (spaceUpdatesToRun.load() > 0) {
		this_thread::sleep_for(chrono::duration<int, milli>(1));
	}

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
	//Since the ChamberCompletedMenu directly accesses gspace for object data,ensure that
	//gspace update is not currently running before creating it.
	while (spaceUpdatesToRun.load() > 0) {
		this_thread::sleep_for(chrono::duration<int, milli>(1));
	}

	showMenu(Node::ccCreate<ChamberCompletedMenu>(this));
}

void PlayScene::triggerSceneCompleted()
{
	setPaused(true);
	triggerMenu(&PlayScene::showSceneCompletedMenu);
}

void PlayScene::showReplayCompletedMenu()
{
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

	//Since the MapMenu directly accesses gspace for object data, ensure that
	//gspace update is not currently running before creating it.
	while (spaceUpdatesToRun.load() > 0) {
		this_thread::sleep_for(chrono::duration<int, milli>(1));
	}

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

	while (spaceUpdatesToRun.load() > 0) {
		this_thread::sleep_for(chrono::duration<int, milli>(1));
	}

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

void PlayScene::initReplayData()
{
	controlReplay = make_unique<ControlReplay>();

	controlReplay->scene_name = GScene::crntSceneName;
}

void PlayScene::updateReplayData()
{
	if (controlReplay && isRunningReplay && gspace->getFrame() >= controlReplay->control_data.size()) {
		triggerReplayCompleted();
	}

	else if (controlReplay && !isRunningReplay) {
		controlReplay->control_data.push_back(App::control_register->getControlState());
	}
}

bool PlayScene::loadReplayData(const string& filename)
{
	if (controlReplay && !isRunningReplay) {
		log("PlayScene::loadReplayData: overwriting existing data.");
	}
	controlReplay = io::getControlReplay(filename);

	isRunningReplay = controlReplay != nullptr;
	return controlReplay != nullptr;
}

void PlayScene::saveReplayData(const string& filename)
{
	if (controlReplay && !isRunningReplay) {
		io::saveControlReplay(filename, controlReplay.get());
	}
}

ControlInfo PlayScene::getControlData()
{
	unsigned int crntFrame = gspace->getFrame();

	if (isRunningReplay){
		if (crntFrame > 0 && crntFrame < controlReplay->control_data.size()) {
			ControlInfo result;

			result.left_v = controlReplay->control_data[crntFrame].left_v;
			result.right_v = controlReplay->control_data[crntFrame].right_v;
			
			result.action_state_crnt = controlReplay->control_data[crntFrame].action_state;
			result.action_state_prev = controlReplay->control_data[crntFrame-1].action_state;

			return result;
		}
		else {
			log("ControlReplay out of bounds, frame: %d", crntFrame);
			return ControlInfo();
		}
	}
	else {
		return App::control_register->getControlInfo();
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
