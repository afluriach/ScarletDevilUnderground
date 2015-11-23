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

class Glyph : public virtual GObject, RectangleBody, ImageSprite
{
public:
    inline Glyph(const cocos2d::ValueMap& args) : GObject(args)
    {
    }
    
    virtual void init();
    
    virtual string imageSpritePath() const {return "sprites/glyph.png";}
    virtual PlayScene::Layer sceneLayer() const {return PlayScene::Layer::ground;}
    
    virtual inline float getMass() const {return 1;}
};

#endif /* Glyph_hpp */
