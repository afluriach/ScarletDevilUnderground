//
//  util.h
//  FlansBasement
//
//  Created by Toni on 10/13/15.
//
//

#ifndef util_h
#define util_h

#include <boost/foreach.hpp>

#define foreach BOOST_FOREACH

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

#endif /* util_h */
