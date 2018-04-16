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

class Wall : virtual public GObject, RectangleBody
{
public:
//    inline Wall(const ValueMap& args) :
//    GObject("wall", SpaceVect(getFloat(args, "pos_x"), getFloat(args, "pos_y"))),
//    dimensions(RectangleMapBody::getDimensionsFromMap(args))
//    {}
    
    inline Wall(SpaceVect center, SpaceVect dimensions) :
    GObject("wall",center),
    dimensions(dimensions)
    {}
    
    virtual inline float getMass() const { return -1.0f;}
    virtual inline GType getType() const {return GType::wall;}
    virtual inline SpaceVect getDimensions() const {return dimensions;}
    
    inline virtual void initializeGraphics(Layer* layer){};
private:
    SpaceVect dimensions;
};

#endif /* Wall_hpp */
