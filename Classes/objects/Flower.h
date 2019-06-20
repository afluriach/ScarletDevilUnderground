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

class Flower : virtual public GObject, CircleBody
{
public:
	Flower(GSpace* space, ObjectIDType id, const string& name, const string& color, const SpaceVect& pos);
	Flower(GSpace* space, ObjectIDType id, const ValueMap& args);
    
    virtual inline string getSprite() const {return "flower_"+color;}

    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::foliage;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::foliage;}
    virtual inline bool getSensor() const {return true;}
    
    virtual inline SpaceFloat getRadius() const {return 0.5;}

    const string color;
};

#endif /* Flower_h */
