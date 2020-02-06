//
//  EnvironmentalObjects.hpp
//  Koumachika
//
//  Created by Toni on 2/23/19.
//
//

#ifndef EnvironmentalObjects_hpp
#define EnvironmentalObjects_hpp

class RectangleSensor;

class Headstone : public GObject
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
    
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
protected:
	static boost::icl::interval_map<float, int> intervals;

	int spriteIdx = 0;
	float hp, maxHP;
};

class GhostHeadstone : public Headstone
{
public:
	MapObjCons(GhostHeadstone);

	virtual void init();

	virtual void removePhysicsObjects();

	void checkActivate();
	void onContact(GObject* obj);
	void onEndContact(GObject* obj);
protected:
	int cost = -1;
	RectangleSensor* sensor = nullptr;
	unordered_set<GObject*> fairies;
};

class Sapling : public GObject
{
public:
	MapObjCons(Sapling);

	virtual string getSprite() const { return "sapling"; }
	virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::all; }
};

#endif /* EnvironmentalObjects_hpp */
