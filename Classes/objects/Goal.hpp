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

class PlayScene;

class Goal:
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public AudioSourceObject,
	public ActivateableObject,
	public InteractibleObject
{
public:
	MapObjCons(Goal);
    
	virtual void update();

    virtual string imageSpritePath() const {return "sprites/goal.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline SpaceFloat getMass() const { return -1.0;}
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	virtual bool canInteract();
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/ui/goal.png"; }

	virtual void activate();
	virtual void deactivate();
protected:
	SpaceFloat audioTimer = 0.0;
	PlayScene * playScene = nullptr;
	bool isBlocked = false;
};

#endif /* Goal_hpp */
