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

//Represents an object that can be collected from the environment.
//It is static, disappears once obtained.
class InventoryObject : public GObject
{
public:
	InventoryObject(GSpace* space, ObjectIDType id, const ValueMap& args);
	InventoryObject(GSpace* space, ObjectIDType id, const ValueMap& args, const physics_params& phys);
	InventoryObject(shared_ptr<object_params> params, const physics_params& phys);

	virtual void init();

	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::floor; }

    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::floor;}
    
    //virtual API for item interaction
	virtual inline bool canAcquire() { return true; }
    virtual inline void onAcquire() {}
    virtual string itemName() const = 0;

	void onPlayerContact();
};

#endif /* InventoryObject_hpp */
