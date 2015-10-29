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

#include "PlayScene.hpp"

#define foreach BOOST_FOREACH
#define list_of boost::assign::list_of
//Convert to concrete container type.
//The list directly follows the list_of macro, and every element include the first
//is in parenthesis
#define list_of_typed(list,type) list_of list .convert_to_container<type>()
#define map_list_of boost::assign::map_list_of

//Despite its name, it could map to another method of a different name
//with the same signature in the same class.
//Otherwise, superMethod identifier includes class name scope.
#define CallSuper(method, superMethod, ret, signature, args) \
inline ret method( signature ) \
{ \
    return superMethod(args); \
} \

#define expand_vector2(v)  ((v).x) , ((v).y) 

//Define assignment conversion between cocos and chipmunk vectors
cocos2d::Vec2 toCocos(const cp::Vect& rhs);
cp::Vect toChipmunk(const cocos2d::Vec2& rhs);

cocos2d::Scene* crntScene();

template <typename T>
cocos2d::Scene* createSceneFromLayer()
{
    cocos2d::Scene* scene  = cocos2d::Scene::create();
    cocos2d::Layer* layer = T::create();
    scene->addChild(layer);
    return scene;
}

template <typename T>
void pushScene()
{
    cocos2d::Director::getInstance()->pushScene(createSceneFromLayer<T>());
}

void end();

void printValueMap(const cocos2d::ValueMap& obj);
float getFloat(const cocos2d::ValueMap& obj, const std::string& name);

cocos2d::Sprite* loadImageSprite(const std::string& resPath, PlayScene::Layer sceneLayer, cocos2d::Layer* dest, const Vec2& pos);

#endif /* util_h */
