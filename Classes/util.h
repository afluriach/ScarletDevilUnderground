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
Vec2 toCocos(const IntVec2& rhs);
SpaceVect toChipmunk(const Vec2& rhs);
SpaceVect toChipmunk(const cocos2d::CCSize& rhs);
IntVec2 toIntVector(const cocos2d::CCSize& rhs);
IntVec2 toIntVector(const SpaceVect& rhs);

SpaceFloat dirToPhysicsAngle(Direction d);
SpaceVect dirToVector(Direction d);
//Cannot be overloaded since it is exposed in the Lua API.
Direction toDirection(SpaceVect);
Direction angleToDirection(SpaceFloat a);
Direction stringToDirection(string str);
SpaceFloat canonicalAngle(SpaceFloat a);

SpaceFloat toDegrees(SpaceFloat a);

SpaceFloat circleMomentOfInertia(SpaceFloat mass, SpaceFloat radius);
SpaceFloat rectangleMomentOfInertia(SpaceFloat mass, const SpaceVect& dim);

void timerDecrement(boost::rational<int>& x);

Scene* crntScene();

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

template<typename T, size_t Size>
inline array<T,Size> getZeroArray()
{
	array<T, Size> result;

	for (int i = 0; i < Size; ++i) {
		result[i] = 0;
	}

	return result;
}

void checkCreateProfileSubfolder();
string getProfilePath();
string getReplayFolderPath();

#endif /* util_h */
