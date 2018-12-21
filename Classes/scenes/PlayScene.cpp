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
		wrap_method(PlayScene, initRoomMask, this),
		to_int(initOrder::initRoomMask)
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
	multiUpdate.insertWithOrder(
		wrap_method(PlayScene, updateRoomMask, this),
		to_int(updateOrder::roomMaskUpdate)
	);

    control_listener->addPressListener(
        ControlAction::pause,
        [=]()-> void {onPausePressed(); }
    );
}

PlayScene::~PlayScene()
{
}

void PlayScene::trackPlayer(){
    cameraTarget = gspace->getObject("player");
}

void PlayScene::initRoomMask()
{
	Layer* layer = getSpaceLayer()->getLayer(GraphicsLayer::roomMask);

	for (size_t i = 0; i<gspace->rooms.size(); ++i)
	{
		CCRect rect = gspace->rooms.at(i);

		DrawNode* dn = DrawNode::create();
		layer->positionAndAddNode(dn, 1, Vec2(rect.getMidX(), rect.getMidY()) * App::pixelsPerTile, 1.0f);

		Vec2 halfDim(rect.getMaxX() - rect.getMinX(), rect.getMaxY() - rect.getMinY());
		halfDim *= App::pixelsPerTile * 0.5f;
		dn->drawSolidRect(-halfDim, halfDim, Color4F(0.0f, 0.0f, 0.0f, 1.0f));

		roomMasks.push_back(dn);
	}
}

void PlayScene::updateRoomMask()
{
	GObject* player = gspace->getObject("player");

	if (gspace->rooms.size() == 0 || !player) {
		return;
	}

	for (size_t i=0;i<gspace->rooms.size(); ++i)
	{
		CCRect rect = gspace->rooms.at(i);

		roomMasks.at(i)->setVisible(!rect.intersectsCircle(toCocos(player->getPos()), player->getRadius()));
	}

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
        move(arrowState * cameraMovePixPerFrame);
    }
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

void PlayScene::enterPause()
{
	pauseMenu = Node::ccCreate<PauseMenu>();
	getLayer(sceneLayers::menu)->addChild(pauseMenu);
    pauseAnimations();
	setPaused(true);
}

void PlayScene::exitPause()
{
	getLayer(sceneLayers::menu)->removeChild(pauseMenu);
	pauseMenu = nullptr;
    resumeAnimations();
	setPaused(false);
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
	showMenu(Node::ccCreate<GameOverMenu>());
}

void PlayScene::triggerGameOver()
{
	spaceLayer->runAction(tintTo(fadeoutColor, fadeoutLength));

	triggerMenu(&PlayScene::showGameOverMenu);
}

void PlayScene::showSceneCompletedMenu()
{
	showMenu(Node::ccCreate<ChamberCompletedMenu>());
}

void PlayScene::triggerSceneCompleted()
{
	triggerMenu(&PlayScene::showSceneCompletedMenu);
}

void PlayScene::showReplayCompletedMenu()
{
	showMenu(Node::ccCreate<ReplayCompletedMenu>());
}

void PlayScene::triggerReplayCompleted()
{
	triggerMenu(&PlayScene::showReplayCompletedMenu);
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
	if (pauseMenu)
		pauseMenu->setVisible(false);

	setPaused(true);

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
