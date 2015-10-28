//
//  util.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

#include "cocos2d.h"

#include <boost/foreach.hpp>
#include <boost/assign.hpp>

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
#define CallSuper(name, method, superMethod, ret, signature, args) \
inline ret method( signature ) \
{ \
    return superMethod(args); \
} \

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

#endif /* util_h */
