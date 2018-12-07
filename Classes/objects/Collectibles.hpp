//
//  Collectibles.hpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#ifndef Collectibles_hpp
#define Collectibles_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Collectible : public virtual GObject
{
public:
	inline Collectible(){}
};

class PowerUp : public Collectible, public RectangleBody, public ImageSprite
{
public:
    inline PowerUp(GSpace* space, ObjectIDType id, const ValueMap& args) :
    MapObjForwarding(GObject)
    {}
    
    virtual inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::collectible;}
    virtual inline bool getSensor() const { return true;}
    virtual inline SpaceVect getDimensions() const {return SpaceVect(0.5f,0.5f);}
    
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::floor;}
    virtual inline string imageSpritePath() const {return "sprites/power_up.png";}
    
};
#endif /* Collectibles_hpp */
