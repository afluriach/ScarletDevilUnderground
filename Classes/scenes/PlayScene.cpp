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
#include "SpaceLayer.h"
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

const Color3B PlayScene::fadeoutColor = Color3B(128,0,0);
const float PlayScene::fadeoutLength = 3.0f;

PlayScene::PlayScene(const string& mapName) :
	PlayScene(mapName, singleMapEntry(mapName))
{}

PlayScene::PlayScene(const string& sceneName, const vector<MapEntry>& maps) :
GScene(sceneName, maps)
{
    multiInit.insertWithOrder(
        wrap_method(PlayScene,trackPlayer,this),
        to_int(initOrder::postLoadObjects)
    );
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
        wrap_method(PlayScene,updateCamera,this),
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
}

PlayScene::~PlayScene()
{
}

void PlayScene::trackPlayer(){
    cameraTarget = gspace->getObject("player");
}

void PlayScene::updateCamera()
{
    if(cameraTarget.isValid()){
        trackCameraTarget();
    }
    else{
        applyCameraControls();
    }
}

void PlayScene::trackCameraTarget()
{
    const SpaceVect& pos = cameraTarget.get()->body->getPos();
    setUnitPosition(pos);
}

void PlayScene::applyCameraControls()
{
    Vec2 arrowState = toCocos(App::control_register->getRightVector());
    if(!arrowState.isZero()){
		Vec2 cameraPos = Vec2(cameraArea.getMidX(), cameraArea.getMidY());

		setUnitPosition(toChipmunk(cameraPos + arrowState * cameraMoveTilesPerSecond * App::secondsPerFrame));
    }
}

bool PlayScene::getSuppressAction()
{
	return isSuppressAction;
}


void PlayScene::setSuppressAction(bool val)
{
	isSuppressAction = val;
}

//Rather than making an updater by capturing hud, just wrap it in a method to access hud from the supplied this.
void PlayScene::updateHUD()
{
    hud->update();
}

void PlayScene::addHUD()
{
    hud = Node::ccCreate<HUD>(gspace, gspace->getObjectRefAs<Player>("player"));
    getLayer(sceneLayers::hud)->addChild(hud);
    multiUpdate.insertWithOrder(
        wrap_method(PlayScene,updateHUD,this),
        to_int(updateOrder::hudUpdate)
    );
}

void PlayScene::onPausePressed()
{
	if (isPaused)
		exitPause();
	else
		enterPause();
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

	pauseMenu = Node::ccCreate<PauseMenu>();
	getLayer(sceneLayers::menu)->addChild(pauseMenu);
    pauseAnimations();
	setPaused(true);
	isShowingMenu = true;
}

void PlayScene::exitPause()
{
	getLayer(sceneLayers::menu)->removeChild(pauseMenu);
	pauseMenu = nullptr;
    resumeAnimations();
	setPaused(false);
	isShowingMenu = false;
}

void PlayScene::pauseAnimations()
{
    spaceLayer->pauseRecursive();
    getLayer(sceneLayers::hud)->pauseRecursive();
}

void PlayScene::resumeAnimations()
{
    spaceLayer->resumeRecursive();
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
	spaceLayer->runAction(tintTo(fadeoutColor, fadeoutLength));

	triggerMenu(&PlayScene::showGameOverMenu);
}

void PlayScene::showSceneCompletedMenu()
{
	showMenu(Node::ccCreate<ChamberCompletedMenu>());
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

	mapMenu = Node::ccCreate<MapMenu>(this);
	getLayer(sceneLayers::menu)->addChild(mapMenu);
	pauseAnimations();
	setPaused(true);
	isShowingMenu = true;
}

void PlayScene::exitMap()
{
	getLayer(sceneLayers::menu)->removeChild(mapMenu);
	mapMenu = nullptr;
	resumeAnimations();
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
	controlReplay = make_unique<ControlReplay>();

	if (controlReplay->load(getReplayFolderPath() + filename + ".replay")) {
		isRunningReplay = true;
		return true;
	}
	return false;
}

bool PlayScene::saveReplayData(const string& filename)
{
	if (controlReplay && !isRunningReplay) {
		return controlReplay->save(getReplayFolderPath() + filename + ".replay");
	}
	else {
		log("PlayScene::saveReplayData: not available.");
		return false;
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

void PlayScene::showMenu(Layer* menu)
{
	hud->setVisible(false);
	if (dialog)
		dialog->setVisible(false);

	getLayer(sceneLayers::menu)->addChild(menu);
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
