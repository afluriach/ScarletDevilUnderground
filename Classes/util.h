//
//  util.h
//  Koumachika
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

#include "types.h"

//Define assignment conversion between cocos and chipmunk vectors
Vec2 toCocos(const SpaceVect& rhs);
SpaceVect toChipmunk(const Vec2& rhs);
SpaceVect toChipmunk(const cocos2d::CCSize& rhs);
IntVec2 toIntVector(const cocos2d::CCSize& rhs);

float dirToPhysicsAngle(Direction d);
SpaceVect dirToVector(Direction d);
//Cannot be overloaded since it is exposed in the Lua API.
Direction toDirection(SpaceVect);
Direction angleToDirection(float a);
Direction stringToDirection(string str);
float canonicalAngle(float a);

float toDegrees(float a);

float circleMomentOfInertia(float mass, float radius);
float rectangleMomentOfInertia(float mass, const SpaceVect& dim);

Scene* crntScene();

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos, float zoom);

Label* createTextLabel(const string& text, int size);

string getRealPath(const string& path);
string loadTextFile(const string& res);
vector<string> splitString(const string& input,const string& sep);

template<typename T>
T vmin(T&&t)
{
  return std::forward<T>(t);
}

template<typename T0, typename T1, typename... Ts>
typename std::common_type<
  T0, T1, Ts...
>::type vmin(T0&& val1, T1&& val2, Ts&&... vs)
{
  if (val2 < val1)
    return vmin(val2, std::forward<Ts>(vs)...);
  else
    return vmin(val1, std::forward<Ts>(vs)...);
}

void checkCreateProfileSubfolder();
string getProfilePath();

#endif /* util_h */
