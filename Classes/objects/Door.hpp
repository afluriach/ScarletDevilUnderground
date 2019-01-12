//
//  Door.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef Door_hpp
#define Door_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Door :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public InteractibleObject
{
public:
	MapObjCons(Door);
    
    virtual inline string imageSpritePath() const {return "sprites/door.png";}
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
	virtual inline float zoom() const { return 0.5f; }

    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

	virtual inline bool canInteract() { return true; }
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/door.png"; }
protected:
	//the angle/offset when this door is used as a destination
	Direction entryDirection;
	string destination;
};
#endif /* Door_hpp */
