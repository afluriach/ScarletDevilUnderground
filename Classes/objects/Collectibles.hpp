//
//  Collectibles.hpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#ifndef Collectibles_hpp
#define Collectibles_hpp

#include "InventoryObject.hpp"

class MagicEffectDescriptor;

enum class collectible_id
{
	nil,

	health1,
	health2,
	health3,

	magic1,
	magic2,
	magic3,

	//random health or magic
	hm1,
	hm2,
	hm3,

	key,
};

struct collectible_properties
{
	string sprite;

	Attribute attr;
	float val;
};

class Collectible : public InventoryObject
{
public:
	static const boost::bimap<collectible_id, string> collectibleNameMap;

	static const unordered_map<collectible_id, collectible_properties> propertiesMap;
	static const unordered_map<collectible_id, function<collectible_id(GSpace*)>> vMap;

	static ObjectGeneratorType create(GSpace* space, collectible_id id, SpaceVect pos);
	static collectible_id getCollectibleID(const string& name);

	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, collectible_id collectibleID);
	inline virtual ~Collectible() {}

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual string getSprite() const;
	virtual string itemName() const;

	shared_ptr<MagicEffectDescriptor> getEffect(GObject* target) const;

	virtual bool canAcquire(Player* player);
	virtual void onAcquire(Player* player);
protected:
	string sprite;
	shared_ptr<MagicEffectDescriptor> effect;
};

#endif /* Collectibles_hpp */
