//
//  PlayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include "App.h"
#include "controls.h"
#include "PlayScene.hpp"
#include "GObject.hpp"
#include "util.h"

using namespace std;
USING_NS_CC;

void printGroup(TMXObjectGroup* group)
{
    const ValueVector& objects = group->getObjects();
    
    foreach(Value obj, objects)
    {
        const ValueMap& objAsMap = obj.asValueMap();
        printValueMap(objAsMap);
    }
}

void PlayScene::updateCamera(float dt)
{
    KeyRegister* kr = app->keyRegister;
    
    //Check camera scroll.
    if(kr->isKeyDown(Keys::up) && !kr->isKeyDown(Keys::down))
        move(0, cameraMovePixPerFrame);
    if(kr->isKeyDown(Keys::down) && !kr->isKeyDown(Keys::up))
        move(0, -cameraMovePixPerFrame);
    if(kr->isKeyDown(Keys::left) && !kr->isKeyDown(Keys::right))
        move(-cameraMovePixPerFrame, 0);
    if(kr->isKeyDown(Keys::right) && !kr->isKeyDown(Keys::left))
        move(cameraMovePixPerFrame, 0);
}

void PlayScene::move(int dx, int dy)
{
    setPosition(getPositionX()-dx, getPositionY()-dy);
}
