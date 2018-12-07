//
//  Goal.hpp
//  Koumachika
//
//  Created by Toni on 12/5/18.
//
//

#ifndef Goal_hpp
#define Goal_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Goal: virtual public GObject, public RectangleBody, public ImageSprite, public InteractibleObject
{
public:
	MapObjCons(Goal);
    
    virtual string imageSpritePath() const {return "sprites/goal.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline float getMass() const { return -1;}
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }
    virtual inline SpaceVect getDimensions() const {return SpaceVect(1,1);}

	virtual bool canInteract();
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/ui/goal.png"; }
};

#endif /* Goal_hpp */
