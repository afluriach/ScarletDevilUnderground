//
//  PlayScene.cpp
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

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

void PlayScene::updateCamera()
{
    if(cameraTarget){
        trackCameraTarget();
    }
    else{
        applyCameraControls();
    }
}

void PlayScene::trackCameraTarget()
{
    const SpaceVect& pos = cameraTarget->body->getPos();
    setUnitPosition(pos);
}

void PlayScene::applyCameraControls()
{
    KeyRegister* kr = app->keyRegister;

    Vec2 arrowState = kr->getArrowKeyState();
    arrowState *= cameraMovePixPerFrame;
    move(arrowState);
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
	setPaused(true);
}

void PlayScene::exitPause()
{
	getLayer(sceneLayers::menu)->removeChild(pauseMenu);
	pauseMenu = nullptr;
	setPaused(false);
}
