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
public RectangleBody
{
public:
	MapObjCons(CollectGlyph);

    void init();
    
    virtual inline string getSprite() const {return "glyph";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);
protected:
	bool hasInteracted = false;
	Collect* collectScene = nullptr;
};


#endif /* CollectGlyph_h */
