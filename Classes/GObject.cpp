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

ValueMap GObject::makeValueMapArg(const Vec2& pos, const Vec2& dim, const map<string,string>& props)
{
    ValueMap vm;
    
    Vec2 posPix = pos * App::pixelsPerTile;
    Vec2 dimPix = dim * App::pixelsPerTile;
    
    vm["x"] = Value(posPix.x);
    vm["y"] = Value(posPix.y);
    
    vm["width"] = Value(dimPix.x);
    vm["height"] = Value(dimPix.y);
    
    for(auto it = props.begin(); it != props.end(); ++it)
    {
        vm[it->first] = it->second;
    }
    
    return vm;
}

void SpriteObject::updateSpritePos()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

cocos2d::Vec2 GObject::getInitialCenterPix()
{
    cp::Vect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
    
    return toCocos(centerPix);
}

void ImageSprite::loadImageSprite(const std::string& resPath, GScene::Layer sceneLayer, cocos2d::Layer* dest)
{
    cocos2d::Vec2 centerPix = getInitialCenterPix();
    sprite = ::loadImageSprite(resPath,sceneLayer,dest, centerPix);
    
    if(sprite == nullptr)
        log("%s sprite not loaded", name.c_str());
    else if(App::logSprites)
        log("%s sprite %s added at %.1f,%.1f, layer %d", name.c_str(), resPath.c_str(), expand_vector2(centerPix), sceneLayer);
}
