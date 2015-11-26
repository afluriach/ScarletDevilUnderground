//
//  util.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

Scene* crntScene()
{
    return Director::getInstance()->getRunningScene();
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

float dirToPhysicsAngle(Direction d)
{
    switch(d){
    case Direction::rightDir: return 0;
    case Direction::upDir: return boost::math::constants::pi<float>() /2;
    case Direction::leftDir: return boost::math::constants::pi<float>();
    case Direction::downDir: return boost::math::constants::pi<float>() *3/2;
    }
}

float getFloat(const cocos2d::ValueMap& args, const std::string& name)
{
    return args.at(name).asFloat();
}

IntVec2 toIntVector(const cocos2d::Size& rhs)
{
    return IntVec2(rhs.width,rhs.height);
}

cocos2d::Vec2 toCocos(const cp::Vect& rhs)
{
    return cocos2d::Vec2(expand_vector2(rhs));
}

cp::Vect toChipmunk(const cocos2d::Vec2& rhs)
{
    return cp::Vect(expand_vector2(rhs));
}

Sprite* loadImageSprite(const std::string& resPath, GraphicsLayer sceneLayer, cocos2d::Layer* dest, const Vec2& pos)
{
    cocos2d::Sprite* node = cocos2d::Sprite::create(resPath);
    
    if(node)
        dest->positionAndAddNode(node,sceneLayer,pos);
    else
        log("loadImageSprite: sprite %s not loaded", resPath.c_str());

    return node;
}
