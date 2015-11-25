//
//  util.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

#include "chipmunk.hpp"
#include "cocos2d.h"

#include <boost/foreach.hpp>
#include <boost/assign.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/math/constants/constants.hpp>

#include "PlayScene.hpp"

using namespace boost::assign;

#define foreach BOOST_FOREACH
//Convert to concrete container type.
//The list directly follows the list_of macro, and every element include the first
//is in parenthesis
#define list_of_typed(list,type) list_of list .convert_to_container<type>()

//Despite its name, it could map to another method of a different name
//with the same signature in the same class.
//Otherwise, superMethod identifier includes class name scope.
#define CallSuper(method, superMethod, ret, signature, args) \
inline ret method( signature ) \
{ \
    return superMethod(args); \
} \

#define no_op(method) inline void method() {}

#define expand_vector2(v)  ((v).x) , ((v).y) 

typedef pair<int,int> IntVec2;

enum Direction{
    rightDir=1,
    upDir,
    leftDir,
    downDir
};


//Define assignment conversion between cocos and chipmunk vectors
cocos2d::Vec2 toCocos(const cp::Vect& rhs);
cp::Vect toChipmunk(const cocos2d::Vec2& rhs);
IntVec2 toIntVector(const cocos2d::Size& rhs);

float dirToPhysicsAngle(Direction d);

cocos2d::Scene* crntScene();

void printValueMap(const cocos2d::ValueMap& obj);
float getFloat(const cocos2d::ValueMap& obj, const std::string& name);

void positionAndAddNode(Node* node, GScene::Layer sceneLayer, cocos2d::Layer* dest, const Vec2& pos);
cocos2d::Sprite* loadImageSprite(const std::string& resPath, GScene::Layer sceneLayer, cocos2d::Layer* dest, const Vec2& pos);

inline std::string loadTextFile(const std::string& res)
{
    return cocos2d::FileUtils::getInstance()->getStringFromFile(res);
}

inline std::vector<std::string> splitString(const std::string& input,const std::string& sep)
{
    std::vector<std::string> output;
    boost::split(output, input,boost::is_any_of(sep));
    return output;
}

#endif /* util_h */
