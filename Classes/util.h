//
//  util.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

//Define assignment conversion between cocos and chipmunk vectors
Vec2 toCocos(const SpaceVect& rhs);
SpaceVect toChipmunk(const Vec2& rhs);
IntVec2 toIntVector(const cocos2d::Size& rhs);

float dirToPhysicsAngle(Direction d);

Scene* crntScene();

void printValueMap(const ValueMap& obj);
float getFloat(const ValueMap& obj, const string& name);

Sprite* loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest, const Vec2& pos);

string getRealPath(const string& path);
string loadTextFile(const string& res);
vector<string> splitString(const string& input,const string& sep);

//Bind this but not the explicit arguments.
template<typename T, typename A1>
function<void(A1)> bindMethod(void (T::* m)(A1), T* This)
{
    return bind(m, This, placeholders::_1);
}

template<typename T, typename A1, typename A2>
function<void(A1,A2)> bindMethod(void (T::* m)(A1,A2), T* This)
{
    return bind(m, This, placeholders::_1, placeholders::_2);
}

#endif /* util_h */
