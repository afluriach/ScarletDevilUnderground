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

class Collect;

class CollectGlyph : public GObject
{
public:
	MapObjCons(CollectGlyph);
    
    virtual inline string getSprite() const {return "glyph";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
    
	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);
protected:
	bool hasInteracted = false;
	Collect* collectScene = nullptr;
};


#endif /* CollectGlyph_h */
