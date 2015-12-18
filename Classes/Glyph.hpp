//
//  Glyph.hpp
//  Koumachika
//
//  Created by Toni on 10/28/15.
//
//

#ifndef Glyph_hpp
#define Glyph_hpp

class Glyph : public virtual GObject, RectangleBody, ImageSprite, RegisterInit<Glyph>
{
public:
    inline Glyph(const ValueMap& args) : GObject(args), RegisterInit<Glyph>(this)
    {
    }
    
    void init();
    
    virtual string imageSpritePath() const {return "sprites/glyph.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline float getMass() const {return 1;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}
};

#endif /* Glyph_hpp */
