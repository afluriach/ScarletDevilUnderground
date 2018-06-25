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
//It is static, interactible, and disappears once obtained.
class InventoryObject : virtual public GObject, public CircleBody, public ImageSprite, public InteractibleObject
{
public:
    virtual inline float getRadius() const {return 0.5;}
    virtual inline float getMass() const {return -1;}
    virtual inline GType getType() const {return GType::environment;}
    
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::floor;}

    virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::floor;}
    
    //virtual API for item interaction
    virtual bool canAcquire() = 0;
    virtual void onAcquire() {}
    virtual string itemName() const = 0;
    
    inline virtual bool canInteract(){
        return canAcquire();
    }
    
    virtual void interact();
        
    inline virtual string interactionIcon(){
        return "sprites/ui/item.png";
    }
};

#endif /* InventoryObject_hpp */
