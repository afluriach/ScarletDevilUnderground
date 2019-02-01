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

	virtual inline bool canInteract() { return !locked; }
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/door.png"; }

	void setLocked(bool b);
	Door* getDestination();
	SpaceVect getEntryPosition();
	Direction getEntryDirection();
protected:
	//the angle/offset when this door is used as a destination
	Direction entryDirection;
	string destination;
	bool locked = false;
};

class LockedDoor : public Door
{
public:
	MapObjCons(LockedDoor);

	virtual bool canInteract();
	virtual void interact();
	virtual string interactionIcon();

	inline bool isKeyUsed() const { return keyUsed; }
protected:
	bool keyUsed = false;
};

#endif /* Door_hpp */
