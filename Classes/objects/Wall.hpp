//
//  Wall.hpp
//  Koumachika
//
//  Created by Toni on 4/6/18.
//
//

#ifndef Wall_hpp
#define Wall_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Wall : virtual public GObject, public RectangleBody, public NoSprite
{
public:
//    inline Wall(const ValueMap& args) :
//    GObject("wall", SpaceVect(getFloat(args, "pos_x"), getFloat(args, "pos_y"))),
//    dimensions(RectangleMapBody::getDimensionsFromMap(args))
//    {}
    
    inline Wall(GSpace* space, ObjectIDType id, SpaceVect center, SpaceVect dimensions) :
    GObject(space,id,"wall",center,true),
    dimensions(dimensions)
    {}
    
    virtual inline float getMass() const { return -1.0f;}
    virtual inline GType getType() const {return GType::wall;}
    virtual inline SpaceVect getDimensions() const {return dimensions;}
    virtual PhysicsLayers getLayers() const;    
private:
    SpaceVect dimensions;
};

#endif /* Wall_hpp */
