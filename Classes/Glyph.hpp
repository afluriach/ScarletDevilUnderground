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

class Glyph : public virtual GObject, RectangleBody
{
public:
    inline Glyph(const cocos2d::ValueMap& args) : GObject(args)
    {
        mass = 1;
    }
    
    virtual void init();
    virtual void initializeGraphics(cocos2d::Layer* layer);
};

#endif /* Glyph_hpp */
