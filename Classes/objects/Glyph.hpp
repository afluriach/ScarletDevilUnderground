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
#include "GObjectMixins.hpp"

class Glyph :
	public virtual GObject,
	public RectangleBody,
	public ImageSprite,
	public RegisterInit<Glyph>
{
public:
	MapObjCons(Glyph);
    
    void init();
    
    virtual string getSprite() const {return "glyph";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const {return 40.0;}
    virtual inline GType getType() const {return GType::environment;}
};

#endif /* Glyph_hpp */
