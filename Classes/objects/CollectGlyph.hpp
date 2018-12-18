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
#include "GObjectMixins.hpp"

class Collect;

class CollectGlyph :
public virtual GObject,
public RectangleBody,
public ImageSprite,
public InteractibleObject
{
public:
	MapObjCons(CollectGlyph);

    void init();
    
    virtual inline float zoom() const {return 0.75;}
    virtual inline string imageSpritePath() const {return "sprites/glyph.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

	virtual bool canInteract();
	virtual void interact();
	virtual string interactionIcon();
protected:
	bool hasInteracted = false;
	Collect* collectScene = nullptr;
};


#endif /* CollectGlyph_h */
