//
//  util.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "cocos2d.h"

#include "util.h"

USING_NS_CC;

Scene* crntScene()
{
    return Director::getInstance()->getRunningScene();
}

void end()
{
    Director::getInstance()->end();
}

void printValueMap(const ValueMap& obj)
{
    foreach(ValueMap::value_type entryPair, obj)
    {
        log(
            "%s: %s.",
            entryPair.first.c_str(),
            entryPair.second.asString().c_str()
        );
    }
}