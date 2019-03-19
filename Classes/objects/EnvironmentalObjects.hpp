//
//  EnvironmentalObjects.hpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#ifndef EnvironmentalObjects_hpp
#define EnvironmentalObjects_hpp

#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Headstone :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapObjCons(Headstone);
	virtual inline ~Headstone() {}
    
    virtual string imageSpritePath() const {return "sprites/headstone.png";}
    virtual GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }
};

class GhostHeadstone : public Headstone, public RegisterInit<GhostHeadstone>
{
public:
	MapObjCons(GhostHeadstone);

	void init();
};

class Sapling :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite
{
public:
	MapObjCons(Sapling);

	virtual string imageSpritePath() const { return "sprites/sapling.png"; }
	virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline GType getType() const { return GType::environment; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }
};

class Mushroom :
	virtual public GObject,
	public RectangleBody,
	public ImageSprite,
	public InteractibleObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapObjCons(Mushroom);

	virtual string imageSpritePath() const { return "sprites/mushroom.png"; }
	virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline GType getType() const { return GType::environment; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }

	virtual inline bool canInteract() { return true; }
	virtual void interact();
	virtual inline string interactionIcon() { return "sprites/mushroom.png"; }

protected:
	int objectID;
};


#endif /* EnvironmentalObjects_hpp */
