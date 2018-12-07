//
//  CollectGlyph.hpp
//  Koumachika
//
//  Created by Toni on 12/12/15.
//
//

#ifndef CollectGlyph_h
#define CollectGlyph_h

#include "GObject.hpp"

class CollectGlyph : public virtual GObject, RectangleBody, ImageSprite
{
public:
    inline CollectGlyph(GSpace* space, ObjectIDType id, const ValueMap& args) :
	MapObjForwarding(GObject)
    {}
    
    void init();
    
    virtual inline float zoom() const {return 0.75;}
    virtual inline string imageSpritePath() const {return "sprites/glyph.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}
};


#endif /* CollectGlyph_h */
