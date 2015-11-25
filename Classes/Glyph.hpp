//
//  Glyph.hpp
//  FlansBasement
//
//  Created by Toni on 10/28/15.
//
//

#ifndef Glyph_hpp
#define Glyph_hpp

#include "GObject.hpp"
#include "util.h"

class Glyph : public virtual GObject, RectangleMapBody, ImageSprite
{
public:
    inline Glyph(const cocos2d::ValueMap& args) : GObject(args), RectangleMapBody(args)
    {
    }
    
    virtual void init();
    
    virtual string imageSpritePath() const {return "sprites/glyph.png";}
    virtual GScene::Layer sceneLayer() const {return GScene::Layer::ground;}
    
    virtual inline float getMass() const {return 1;}
    virtual inline GSpace::Type getType() const {return GSpace::Type::environment;}
};

#endif /* Glyph_hpp */
