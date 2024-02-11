//
//  Door.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef Door_hpp
#define Door_hpp

class Door : public GObject
{
public:
	enum class door_type
	{
		pair,
		one_way_source,
		one_way_destination,
	};

	MapObjCons(Door);

	virtual void init();
	
	virtual inline shared_ptr<sprite_properties> getSprite() const { return app::getSprite("door"); }
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}

    virtual PhysicsLayers getLayers() const;

	virtual bool canInteract(Player* p);
	virtual void interact(Player* p);
	virtual string interactionIcon(Player* p);

	virtual void activate();
	virtual void deactivate();
	void setLocked(bool b);

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

	bool canUnlock(Player* p) const;
	bool unlock(Player* p);

	inline bool isOnewayDoor() const {
		return doorType == door_type::one_way_destination || doorType == door_type::one_way_source;
	}
protected:
	//the angle/offset when this door is used as a destination
	Direction entryDirection;
	string keyItem;
	string destination;
	string destinationMap;
	gobject_ref adjacent;
	door_type doorType;
	bool locked = false;
	
	bool stairs = false;
	bool path = false;
};

#endif /* Door_hpp */
