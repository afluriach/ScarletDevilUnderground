//
//  Desk.hpp
//  Koumachika
//
//  Created by Toni on 2/20/19.
//
//

#ifndef Desk_hpp
#define Desk_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Desk :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public InteractibleObject
{
public:
	MapObjCons(Desk);
    
    virtual string imageSpritePath() const {return "sprites/desk.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }
	virtual inline SpaceFloat getMass() const { return -1.0; }

	virtual inline string interactionIcon() { return "sprites/desk.png"; }
	virtual inline bool canInteract() { return true; }
	virtual inline void interact();
};
#endif /* Block_hpp */
