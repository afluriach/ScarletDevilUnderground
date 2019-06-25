//
//  Glyph.hpp
//  Koumachika
//
//  Created by Toni on 10/28/15.
//
//

#ifndef Glyph_hpp
#define Glyph_hpp

#include "GObject.hpp"

class Glyph : public GObject
{
public:
	MapObjCons(Glyph);
    
    virtual string getSprite() const {return "glyph";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
};

#endif /* Glyph_hpp */
