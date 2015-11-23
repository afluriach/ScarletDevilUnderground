//
//  GObject.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "cocos2d.h"

#include "App.h"
#include "PlayScene.hpp"
#include "GObject.hpp"
#include "util.h"

USING_NS_CC;

GObject::GObject(const ValueMap& obj) : name(obj.at("name").asString() )
{
    log("%s created at %.1f,%.1f.", name.c_str(),getFloat(obj, "x")/App::pixelsPerTile, getFloat(obj, "y")/App::pixelsPerTile);
    
    cp::Vect cornerPos(getFloat(obj, "x"), getFloat(obj, "y"));
    cornerPos *= App::tilesPerPixel;
    
    dim = cp::Vect(getFloat(obj, "width"), getFloat(obj, "height"));
    dim *= App::tilesPerPixel;
    
    initialCenter = cp::Vect(cornerPos);
    initialCenter += (dim*0.5);
}


GObject* GObject::constructByType(const std::string& type, const cocos2d::ValueMap& args )
{
    auto it = adapters.find(type);
    
    if(it != adapters.end()){
        AdapterType adapter =  it->second;
        return adapter(args);
    }
    else return nullptr;
}

void GObject::updateSpritePos()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

void ImageSprite::loadImageSprite(const std::string& resPath, PlayScene::Layer sceneLayer, cocos2d::Layer* dest)
{
    cp::Vect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
    
    sprite = ::loadImageSprite(resPath,sceneLayer,dest, toCocos(centerPix));
    
    if(sprite == nullptr)
        log("%s sprite not loaded", name.c_str());
    else if(App::logSprites)
        log("%s sprite %s added at %.1f,%.1f, layer %d", name.c_str(), resPath.c_str(), expand_vector2(centerPix), sceneLayer);
}
