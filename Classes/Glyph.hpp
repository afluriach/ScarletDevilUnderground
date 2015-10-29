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

class Glyph : public GObject
{
public:
    inline Glyph(const cocos2d::ValueMap& args) : GObject(args)
    {
        mass = 1;
    }
    
    CallSuper(
        initializeBody,
        GObject::initRectangleBody,
        std::shared_ptr<cp::Body>,
        cp::Space& space,
        space
    )
    
    virtual void initializeGraphics(cocos2d::Layer* layer);
};

#endif /* Glyph_hpp */
