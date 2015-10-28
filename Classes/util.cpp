//
//  util.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "cocos2d.h"
#include "chipmunk.hpp"

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

float getFloat(const cocos2d::ValueMap& args, const std::string& name)
{
    return args.at(name).asFloat();
}

cocos2d::Vec2 toCocos(const cp::Vect& rhs)
{
    return cocos2d::Vec2(expand_vector2(rhs));
}

cp::Vect toChipmunk(const cocos2d::Vec2& rhs)
{
    return cp::Vect(expand_vector2(rhs));
}
