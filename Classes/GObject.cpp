//
//  GObject.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

#include "PlayScene.hpp"
#include "GObject.hpp"

USING_NS_CC;

GObject::GObject(const ValueMap& obj) : name(obj.at("name").asString() )
{
    
    if(obj.find(Lua::lauArgTag) != obj.end()){
        //This is coming from the scripting API
        
        //Interpret coordinates as center, unit space.
        initialCenter = cp::Vect(getFloat(obj, "x"), getFloat(obj, "y"));
    }
    else{
        //When loaded from a map, coordinates represent the corner in pixels.
        cp::Vect cornerPos(getFloat(obj, "x"), getFloat(obj, "y"));
        cornerPos *= App::tilesPerPixel;
        
        cp::Vect dim(getFloat(obj, "width"), getFloat(obj, "height"));
        dim *= App::tilesPerPixel;
        
        initialCenter = cp::Vect(cornerPos);
        initialCenter += (dim*0.5);
    }
    
    log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);
}

GObject::GObject(const string& name, const cp::Vect& pos) : name(name), initialCenter(pos) {

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

ValueMap GObject::makeValueMapArg(const Vec2& pos, const map<string,string>& props)
{
    ValueMap vm;
    
    vm["x"] = Value(pos.x);
    vm["y"] = Value(pos.y);
    
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

cp::Vect RectangleMapBody::getDimensionsFromMap(const ValueMap& arg)
{
    return cp::Vect(getFloat(arg, "width")*App::tilesPerPixel, getFloat(arg, "height")*App::tilesPerPixel);
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
