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
	public RectangleBody
{
public:
	static const vector<string> damageSprites;
	static const vector<float_pair> damageIntervals;

	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapObjCons(Headstone);
	virtual inline ~Headstone() {}
    
	void hit(float damage);

	virtual string getSprite() const;
    virtual inline GraphicsLayer sceneLayer() const {return GraphicsLayer::ground;}
    
    virtual inline GType getType() const {return GType::environment;}
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }
protected:
	static boost::icl::interval_map<float, int> intervals;

	int spriteIdx = 0;
	float hp, maxHP;
};

class GhostHeadstone : public Headstone, public RegisterInit<GhostHeadstone>
{
public:
	MapObjCons(GhostHeadstone);

	void init();
};

class Sapling :
	virtual public GObject,
	public RectangleBody
{
public:
	MapObjCons(Sapling);

	virtual string getSprite() const { return "sapling"; }
	virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline GType getType() const { return GType::environment; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }
};

class Mushroom :
	virtual public GObject,
	public RectangleBody,
	public InteractibleObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapObjCons(Mushroom);

	virtual string getSprite() const { return "mushroom"; }
	virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline GType getType() const { return GType::environment; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
	virtual inline SpaceFloat getMass() const { return -1.0; }

	virtual inline bool canInteract(Player* p) { return true; }
	virtual void interact(Player* p);
	virtual inline string interactionIcon(Player* p) { return "sprites/mushroom.png"; }

protected:
	int objectID;
};


#endif /* EnvironmentalObjects_hpp */
