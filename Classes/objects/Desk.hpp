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

class Desk : public GObject
{
public:
	MapObjCons(Desk);
    
    virtual string getSprite() const {return "desk";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	virtual inline string interactionIcon(Player* p) { return "sprites/desk.png"; }
	virtual inline bool canInteract(Player* p) { return true; }
	virtual inline void interact(Player* p);
};
#endif /* Block_hpp */
