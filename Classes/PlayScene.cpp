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
#include "GObject.hpp"
#include "GSpace.hpp"
#include "macros.h"
#include "multifunction.h"
#include "PlayScene.hpp"

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        printValueMap(objAsMap);
    }
}

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
    multiUpdate.insertWithOrder(
        wrap_method(PlayScene,updateCamera,this),
        static_cast<int>(updateOrder::moveCamera)
    );

    control_listener->addPressListener(
        ControlAction::pause,
        [=]()-> void {onPausePressed(); }
    );
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
