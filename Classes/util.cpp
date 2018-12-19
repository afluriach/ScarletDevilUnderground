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

const SpaceFloat primaryAngles[4] = {0.0, float_pi * 0.5, float_pi, float_pi * 1.5};

Scene* crntScene()
{
    return Director::getInstance()->getRunningScene();
}


SpaceFloat dirToPhysicsAngle(Direction d)
{
    if(d == Direction::none) return 0.0;
    
    return primaryAngles[to_int(d)-1];
}

SpaceVect dirToVector(Direction d)
{
    return SpaceVect::ray(1.0, dirToPhysicsAngle(d));
}

//cocos Vector uses atan2, which returns angle in range [-pi,pi]
Direction toDirection(SpaceVect v)
{
    if(v.x == 0.0 && v.y == 0.0)
        return Direction::none;
    
    return angleToDirection(toCocos(v).getAngle());
}

//round to nearest primary direction
Direction angleToDirection(SpaceFloat a)
{
    int closest = 0;
    SpaceFloat distance = float_pi;
    SpaceFloat angle = canonicalAngle(a);
    
    for_irange(i,0,4){
        SpaceFloat crnt = abs(angle - primaryAngles[i]);
        
        if(crnt < distance){
            closest = i;
            distance = crnt;
        }
    }
    
    return static_cast<Direction>(closest+1);
}

SpaceFloat circleMomentOfInertia(SpaceFloat mass, SpaceFloat radius)
{
    return float_pi/2*pow(radius,4);
}

SpaceFloat rectangleMomentOfInertia(SpaceFloat mass, const SpaceVect& dim)
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


IntVec2 toIntVector(const cocos2d::CCSize& rhs)
{
    return IntVec2(rhs.width,rhs.height);
}

IntVec2 toIntVector(const SpaceVect& rhs)
{
	return IntVec2(rhs.x, rhs.y);
}


Vec2 toCocos(const SpaceVect& rhs)
{
    return Vec2(expand_vector2(rhs));
}

Vec2 toCocos(const IntVec2& rhs)
{
	return Vec2(rhs.first, rhs.second);
}


SpaceVect toChipmunk(const Vec2& rhs)
{
    return SpaceVect(expand_vector2(rhs));
}

SpaceVect toChipmunk(const cocos2d::CCSize& rhs)
{
    return SpaceVect(rhs.width, rhs.height);
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

string getReplayFolderPath()
{
	return FileUtils::getInstance()->getWritablePath() + "koumachika/replay/";
}

SpaceFloat canonicalAngle(SpaceFloat a)
{
    return a - float_2pi * floor( a / float_2pi);
}

SpaceFloat toDegrees(SpaceFloat a)
{
	return a / float_pi * 180.0;
}

void timerDecrement(boost::rational<int>& x)
{
	x = max(x - App::secondsPerFrameRational, boost::rational<int>(0));
}
