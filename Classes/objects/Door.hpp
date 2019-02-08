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
	public ActivateableObject,
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

	virtual bool canInteract();
	virtual void interact();
	virtual string interactionIcon();

	virtual void activate();
	virtual void deactivate();
	void setSealed(bool b);

	Door* getDestination();
	SpaceVect getEntryPosition();
	Direction getEntryDirection();

	inline bool isLocked() const { return locked; }
protected:
	//the angle/offset when this door is used as a destination
	Direction entryDirection;
	string destination;
	bool sealed = false;
	bool locked = false;
};

#endif /* Door_hpp */
