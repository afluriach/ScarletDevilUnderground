//
//  util.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

const string defaultFont = "Arial";

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
    case Direction::noneDir:
    case Direction::rightDir: return 0;
    case Direction::upDir: return float_pi /2;
    case Direction::leftDir: return float_pi;
    case Direction::downDir: return float_pi *3/2;
    }
}

//cocos Vector uses atan2, which returns angle in range [-pi,pi]
Direction toDirection(SpaceVect v)
{
    if(v.x == 0 && v.y == 0)
        return Direction::noneDir;

    float a = toCocos(v).getAngle();
    
    if(a >= -1.0/4.0*float_pi && a < 1.0/4.0*float_pi)
        return Direction::rightDir;

    if(a >=1.0/4.0*float_pi && a < 3.0/4.0*float_pi)
        return Direction::upDir;
    if(a >= -3.0/4.0*float_pi && a < -1.0/4.0*float_pi)
        return Direction::downDir;
    
    if(a >= 3.0/4.0*float_pi || a < -3.0/4.0*float_pi)
        return Direction::leftDir;
    
    //shouldn't happen
    return Direction::noneDir;
}

float getFloat(const ValueMap& args, const string& name)
{
    return args.at(name).asFloat();
}

float getFloatOrDefault(const ValueMap& args, const string& name, float def)
{
    if(args.find(name) != args.end())
        return getFloat(args,name);
    else return def;
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

SpaceVect toChipmunk(const cocos2d::Size& rhs)
{
    return SpaceVect(rhs.width, rhs.height);
}


Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom)
{
    Sprite* node = Sprite::create(resPath);
    
    if(node)
        dest->positionAndAddNode(node,sceneLayer,pos, zoom);
    else
        log("loadImageSprite: sprite %s not loaded", resPath.c_str());

    return node;
}

Label* createTextLabel(const string& text, int size)
{
    Label *l = Label::createWithSystemFont(text, defaultFont, size);
    return l;
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

void convertToUnitSpace(ValueMap& arg)
{
    SpaceVect cornerPos(getFloat(arg, "x"), getFloat(arg, "y"));
    cornerPos *= App::tilesPerPixel;
    
    SpaceVect dim(getFloat(arg, "width"), getFloat(arg, "height"));
    dim *= App::tilesPerPixel;
    
    SpaceVect center = SpaceVect(cornerPos);
    center += (dim*0.5);
    
    arg.erase("x");
    arg.erase("y");

    arg.erase("width");
    arg.erase("height");
    
    arg["pos_x"] = Value(center.x);
    arg["pos_y"] = Value(center.y);
    
    arg["dim_x"] = Value(dim.x);
    arg["dim_y"] = Value(dim.y);
}

cocos2d::Rect getUnitspaceRectangle(const ValueMap& tileMapObj)
{
    SpaceVect cornerPos(getFloat(tileMapObj, "x"), getFloat(tileMapObj, "y"));
    cornerPos *= App::tilesPerPixel;
    
    SpaceVect dim(getFloat(tileMapObj, "width"), getFloat(tileMapObj, "height"));
    dim *= App::tilesPerPixel;
    
    return cocos2d::Rect(cornerPos.x, cornerPos.y, dim.x, dim.y);
}

void checkCreateProfileSubfolder()
{
    auto* fu = FileUtils::getInstance();
    
    if(!fu->isDirectoryExist(fu->getWritablePath() + "koumachika")){
        fu->createDirectory(fu->getWritablePath() + "koumachika");
        log("profile directory created");
    }
}

string getProfilePath()
{
    return FileUtils::getInstance()->getWritablePath() + "koumachika/profile.dat";
}