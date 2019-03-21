//
//  InventoryObject.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef InventoryObject_hpp
#define InventoryObject_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

//Represents an object that can be collected from the environment.
//It is static, disappears once obtained.
class InventoryObject : virtual public GObject, public CircleBody, public ImageSprite
{
public:
    virtual inline SpaceFloat getRadius() const {return 0.5;}
    virtual inline SpaceFloat getMass() const {return -1.0;}
    virtual inline GType getType() const {return GType::playerPickup;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::floor; }

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::floor;}
    
    //virtual API for item interaction
    virtual bool canAcquire() = 0;
    virtual void onAcquire() {}
    virtual string itemName() const = 0;

	void onPlayerContact();
};

#endif /* InventoryObject_hpp */
