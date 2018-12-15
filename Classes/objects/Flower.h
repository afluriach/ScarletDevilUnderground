//
//  Flower.h
//  Koumachika
//
//  Created by Toni on 12/1/15.
//
//

#ifndef Flower_h
#define Flower_h

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Flower : virtual public GObject, CircleBody, ImageSprite
{
public:
    inline Flower(GSpace* space, ObjectIDType id, const string& name, const string& color, const SpaceVect& pos):
    GObject(space,id,name, pos, true),
    color(color){
    }
    inline Flower(GSpace* space, ObjectIDType id, const ValueMap& args):
    GObject(space,id,args),
    color(args.at("color").asString()){
    }    
    
    inline string imageSpritePath() const {return "sprites/flower "+color+".png";}

    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::foliage;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::foliage;}
    virtual inline bool getSensor() const {return true;}
    
    virtual inline SpaceFloat getRadius() const {return 0.5;}

    const string color;
};

#endif /* Flower_h */
