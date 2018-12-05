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
#include "PlayScene.hpp"
#include "value_map.hpp"

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        printValueMap(objAsMap);
    }
}

const Color3B PlayScene::fadeoutColor = Color3B(128,0,0);
const float PlayScene::fadeoutLength = 3.0f;

PlayScene::PlayScene(const string& name) :
MapScene(name),
ScriptedScene(name)
{
    multiInit.insertWithOrder(
        wrap_method(PlayScene,trackPlayer,this),
        static_cast<int>(initOrder::postLoadObjects)
    );
    multiInit.insertWithOrder(
        wrap_method(PlayScene,addHUD,this),
        static_cast<int>(initOrder::initHUD)
    );
	multiInit.insertWithOrder(
		wrap_method(PlayScene, initRoomMask, this),
		static_cast<int>(initOrder::initRoomMask)
	);

    multiUpdate.insertWithOrder(
        wrap_method(PlayScene,updateCamera,this),
        static_cast<int>(updateOrder::moveCamera)
    );
	multiUpdate.insertWithOrder(
		wrap_method(PlayScene, updateRoomMask, this),
		static_cast<int>(updateOrder::roomMaskUpdate)
	);

    control_listener->addPressListener(
        ControlAction::pause,
        [=]()-> void {onPausePressed(); }
    );
    
    app->playScene = this;
}

void PlayScene::trackPlayer(){
    cameraTarget = gspace->getObject("player");
}

void PlayScene::initRoomMask()
{
	Layer* layer = GScene::crntScene->getLayer(GScene::sceneLayers::space);

	for (int i = 0; i<gspace->rooms.size(); ++i)
	{
		CCRect rect = gspace->rooms.at(i);

		DrawNode* dn = DrawNode::create();
		layer->positionAndAddNode(dn, static_cast<int>(GraphicsLayer::roomMask), Vec2(rect.getMidX(), rect.getMidY()) * App::pixelsPerTile, 1.0f);

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

	for (int i=0;i<gspace->rooms.size(); ++i)
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
    Vec2 arrowState = toCocos(app->control_register->getRightVector());
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
    hud = HUD::create();
    getLayer(sceneLayers::hud)->addChild(hud);
    multiUpdate.insertWithOrder(
        wrap_method(PlayScene,updateHUD,this),
        static_cast<int>(updateOrder::hudUpdate)
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
	pauseMenu = PauseMenu::create();
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
    getLayer(sceneLayers::space)->pauseRecursive();
    getLayer(sceneLayers::hud)->pauseRecursive();
}

void PlayScene::resumeAnimations()
{
    getLayer(sceneLayers::space)->resumeRecursive();
    getLayer(sceneLayers::hud)->resumeRecursive();
}

void PlayScene::showGameOverMenu(float unused)
{
    app->hud->setVisible(false);
	if(app->dialog)
        app->dialog->setVisible(false);

	GameOverMenu* gom = GameOverMenu::create();

    getLayer(sceneLayers::menu)->addChild(gom);
}

void PlayScene::triggerGameOver()
{
    if(pauseMenu)
        pauseMenu->setVisible(false);
    
    setPaused(true);
    
    getLayer(sceneLayers::space)->runAction(tintTo(fadeoutColor,fadeoutLength));

    Director::getInstance()->getScheduler()->schedule(
        bind(&PlayScene::showGameOverMenu, this, placeholders::_1),
        this,
        0.0f,
        0,
        fadeoutLength,
        false,
        "showGameOverMenu"
    );
}

void PlayScene::showSceneCompletedMenu(float unused)
{
	app->hud->setVisible(false);
	if (app->dialog)
		app->dialog->setVisible(false);

	ChamberCompletedMenu* menu = ChamberCompletedMenu::create();

	getLayer(sceneLayers::menu)->addChild(menu);
}

void PlayScene::triggerSceneCompleted()
{
	if (pauseMenu)
		pauseMenu->setVisible(false);

	setPaused(true);

	Director::getInstance()->getScheduler()->schedule(
		bind(&PlayScene::showSceneCompletedMenu, this, placeholders::_1),
		this,
		0.0f,
		0,
		fadeoutLength,
		false,
		"showSceneCompletedMenu"
	);
}