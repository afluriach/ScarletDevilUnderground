//
//  util.cpp
//  Koumachika
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "App.h"
#include "macros.h"
#include "util.h"

const string defaultFont = "Arial";

const float primaryAngles[4] = {0.0f, float_pi * 0.5f, float_pi, float_pi * 1.5f};

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

bool validateStringArg(const ValueMap& args, string name)
{
    auto it = args.find(name);
    
    return ( it != args.end() && args.at(name).isString() && args.at(name).asString() != "" );
}

float dirToPhysicsAngle(Direction d)
{
    if(d == Direction::none) return 0.0f;
    
    return primaryAngles[static_cast<int>(d)-1];
}

SpaceVect dirToVector(Direction d)
{
    return SpaceVect::ray(1.0f, dirToPhysicsAngle(d));
}

//cocos Vector uses atan2, which returns angle in range [-pi,pi]
Direction toDirection(SpaceVect v)
{
    if(v.x == 0 && v.y == 0)
        return Direction::none;
    
    return angleToDirection(toCocos(v).getAngle());
}

//round to nearest primary direction
Direction angleToDirection(float a)
{
    int closest = 0;
    float distance = float_pi;
    float angle = canonicalAngle(a);
    
    for_irange(i,0,4){
        float crnt = abs(angle - primaryAngles[i]);
        
        if(crnt < distance){
            closest = i;
            distance = crnt;
        }
    }
    
    return static_cast<Direction>(closest+1);
}

float circleMomentOfInertia(float mass, float radius)
{
    return float_pi/2*pow(radius,4);
}

float rectangleMomentOfInertia(float mass, const SpaceVect& dim)
{
    return mass*(dim.x*dim.x+dim.y*dim.y)/12;
}

#define enum_strcmp(val) if(str == #val) return Direction::val;
Direction stringToDirection(string str)
{
    enum_strcmp(up)
    enum_strcmp(right)
    enum_strcmp(left)
    enum_strcmp(down)
    
    log("Invalid direction: %s", str.c_str());
    return Direction::none;
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

IntVec2 toIntVector(const cocos2d::CCSize& rhs)
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

SpaceVect toChipmunk(const cocos2d::CCSize& rhs)
{
    return SpaceVect(rhs.width, rhs.height);
}


Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom)
{
    Sprite* node = Sprite::create(resPath);
    
    if(node)
        dest->positionAndAddNode(node,static_cast<int>(sceneLayer),pos, zoom);
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

cocos2d::CCRect getUnitspaceRectangle(const ValueMap& tileMapObj)
{
    SpaceVect cornerPos(getFloat(tileMapObj, "x"), getFloat(tileMapObj, "y"));
    cornerPos *= App::tilesPerPixel;
    
    SpaceVect dim(getFloat(tileMapObj, "width"), getFloat(tileMapObj, "height"));
    dim *= App::tilesPerPixel;
    
    return cocos2d::CCRect(cornerPos.x, cornerPos.y, dim.x, dim.y);
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

float canonicalAngle(float a)
{
    return a - float_2pi * floor( a / float_2pi);
}

float toDegrees(float a)
{
 return a / float_pi * 180.0f;
}
