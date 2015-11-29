//
//  GObject.cpp
//  FlansBasement
//
//  Created by Toni on 10/27/15.
//
//

#include "Prefix.h"

unsigned int GObject::nextUUID = 1;

GObject::GObject(const ValueMap& obj) : name(obj.at("name").asString()), uuid(nextUUID++)
{
    if(obj.find(Lua::lauArgTag) != obj.end()){
        //This is coming from the scripting API
        
        //Interpret coordinates as center, unit space.
        initialCenter = SpaceVect(getFloat(obj, "x"), getFloat(obj, "y"));
    }
    else{
        //When loaded from a map, coordinates represent the corner in pixels.
        SpaceVect cornerPos(getFloat(obj, "x"), getFloat(obj, "y"));
        cornerPos *= App::tilesPerPixel;
        
        SpaceVect dim(getFloat(obj, "width"), getFloat(obj, "height"));
        dim *= App::tilesPerPixel;
        
        initialCenter = SpaceVect(cornerPos);
        initialCenter += (dim*0.5);
    }
    
    log("%s created at %.1f,%.1f.", name.c_str(),initialCenter.x, initialCenter.y);
}

GObject::GObject(const string& name, const SpaceVect& pos) : name(name), initialCenter(pos), uuid(nextUUID++) {
}

GObject* GObject::constructByType(const string& type, const ValueMap& args )
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

void SpriteObject::update()
{
    if(sprite != nullptr){
        sprite->setPosition(toCocos(body->getPos())*App::pixelsPerTile);
    }
}

void LoopAnimationSprite::initializeGraphics(Layer* layer)
{
    anim = TimedLoopAnimation::create();
    anim->loadAnimation(animationName(), animationSize(), animationDuration());
    
    layer->positionAndAddNode(anim, sceneLayer(), getInitialCenterPix(), zoom());
    sprite = anim;
}

void PatchConSprite::initializeGraphics(Layer* layer)
{
    animSprite = PatchConAnimation::create();
    animSprite->loadAnimation(imageSpritePath());
    layer->positionAndAddNode(animSprite, sceneLayer(), getInitialCenterPix(), zoom());
    sprite = animSprite;
}

void PatchConSprite::update()
{
    SpaceVect dist = body->getVel()*App::secondsPerFrame;
    
    animSprite->accumulate(dist.length());
}


Vec2 GObject::getInitialCenterPix()
{
    SpaceVect centerPix(initialCenter);
    centerPix *= App::pixelsPerTile;
    
    return toCocos(centerPix);
}

SpaceVect RectangleMapBody::getDimensionsFromMap(const ValueMap& arg)
{
    return SpaceVect(getFloat(arg, "width")*App::tilesPerPixel, getFloat(arg, "height")*App::tilesPerPixel);
}

void ImageSprite::loadImageSprite(const string& resPath, GraphicsLayer sceneLayer, Layer* dest)
{
    Vec2 centerPix = getInitialCenterPix();
    sprite = ::loadImageSprite(resPath,sceneLayer,dest, centerPix, zoom());
    
    if(sprite == nullptr)
        log("%s sprite not loaded", name.c_str());
    else if(App::logSprites)
        log("%s sprite %s added at %.1f,%.1f, layer %d", name.c_str(), resPath.c_str(), expand_vector2(centerPix), sceneLayer);
}
