//
//  PlayScene.cpp
//  FlansBasement
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

void PlayScene::updateCamera(float dt)
{
    KeyRegister* kr = app->keyRegister;

    Vec2 arrowState = kr->getArrowKeyState();
    arrowState *= cameraMovePixPerFrame;
    move(arrowState);
}
