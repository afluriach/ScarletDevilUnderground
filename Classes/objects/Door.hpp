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
	public InteractibleObject,
	public RegisterInit<Door>
{
public:
	enum class door_type
	{
		pair,
		one_way_source,
		one_way_destination,
	};

	MapObjCons(Door);

	void init();
	
	virtual inline string getSprite() const { return "door"; }
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
    virtual PhysicsLayers getLayers() const;

	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);

	virtual void activate();
	virtual void deactivate();
	void setSealed(bool b);

	Door* getAdjacent();
	SpaceVect getEntryPosition();
	Direction getEntryDirection();
	Direction getDoorDirection() const;
	string getDoorDirectionString() const;

	inline string getDestination() const { return destination; }
	inline string getDestinationMap() const { return destinationMap; }
	inline bool isLocked() const { return locked; }
	inline bool isSource() const { return doorType == door_type::one_way_source; }
	inline bool isDestination() const { return doorType == door_type::one_way_destination; }

	inline bool isOnewayDoor() const {
		return doorType == door_type::one_way_destination || doorType == door_type::one_way_source;
	}
protected:
	//the angle/offset when this door is used as a destination
	Direction entryDirection;
	string destination;
	string destinationMap;
	object_ref<Door> adjacent;
	door_type doorType;
	bool sealed = false;
	bool locked = false;
	
	bool stairs = false;
	bool path = false;
};

class Barrier :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public ActivateableObject,
	public RegisterInit<Barrier>
{
public:
	MapObjCons(Barrier);

	virtual inline string getSprite() const { return "barrier"; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::environment; }
	virtual PhysicsLayers getLayers() const;

	virtual void activate();
	virtual void deactivate();

	void setSealed(bool b);
	void init();
protected:
	bool sealed = false;
};

#endif /* Door_hpp */
