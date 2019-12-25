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

struct collectible_properties
{
	string sprite;
	shared_ptr<MagicEffectDescriptor> effect;
	float magnitude;
	float length;
};

class Collectible : public InventoryObject
{
public:
	static ObjectGeneratorType create(GSpace* space, string id, SpaceVect pos);

	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, string collectibleID);
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
	float effectLength;
	float effectMagnitude;
};

#endif /* Collectibles_hpp */
