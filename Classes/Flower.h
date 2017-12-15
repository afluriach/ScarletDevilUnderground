//
//  Flower.h
//  Koumachika
//
//  Created by Toni on 12/1/15.
//
//

#ifndef Flower_h
#define Flower_h

class Flower : virtual public GObject, CircleBody, ImageSprite
{
public:
    inline Flower(const string& name, const string& color, const SpaceVect& pos):
    GObject(name, pos),
    color(color){
    }
    inline Flower(const ValueMap& args):
    GObject(args),
    color(args.at("color").asString()){
    }    
    
    inline string imageSpritePath() const {return "sprites/flower "+color+".png";}

    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::foliage;}
    
    virtual inline float getMass() const { return -1;}
    virtual inline GType getType() const {return GType::foliage;}
    virtual inline bool getSensor() const {return true;}
    
    virtual inline float getRadius() const {return 0.5;}

    const string color;
};

#endif /* Flower_h */
