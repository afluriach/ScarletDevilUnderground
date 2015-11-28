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

float getFloat(const ValueMap& args, const string& name)
{
    return args.at(name).asFloat();
}

IntVec2 toIntVector(const cocos2d::Size& rhs)
{
    return IntVec2(rhs.width,rhs.height);
}

Vec2 toCocos(const SpaceVect& rhs)
{
    return Vec2(expand_vector2(rhs));
}

SpaceVect toChipmunk(const Vec2& rhs)
{
    return SpaceVect(expand_vector2(rhs));
}

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos)
{
    Sprite* node = Sprite::create(resPath);
    
    if(node)
        dest->positionAndAddNode(node,sceneLayer,pos);
    else
        log("loadImageSprite: sprite %s not loaded", resPath.c_str());

    return node;
}

string getRealPath(const string& path)
{
    return FileUtils::getInstance()->fullPathForFilename(path);
}

string loadTextFile(const string& res)
{
    return FileUtils::getInstance()->getStringFromFile(res);
}

vector<string> splitString(const string& input,const string& sep)
{
    vector<string> output;
    boost::split(output, input,boost::is_any_of(sep));
    return output;
}

