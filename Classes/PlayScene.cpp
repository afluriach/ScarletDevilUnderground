//
//  PlayScene.cpp
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#include "Prefix.h"

#include "PlayScene.hpp"

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

    Vec2 arrowState = kr->getArrowKeyState();
    arrowState *= cameraMovePixPerFrame;
    move(arrowState);
}

void PlayScene::move(const Vec2& v)
{
    setPosition(getPositionX()-v.x, getPositionY()-v.y);
}
