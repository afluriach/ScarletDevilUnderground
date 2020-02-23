//
//  Collectibles.hpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#ifndef Collectibles_hpp
#define Collectibles_hpp

class MagicEffectDescriptor;

struct collectible_properties
{
	~collectible_properties();

	string sprite;
	local_shared_ptr<MagicEffectDescriptor> effect;
	effect_attributes attr;
};

class Collectible : public GObject
{
public:
	static ObjectGeneratorType create(GSpace* space, string id, SpaceVect pos);

	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, string collectibleID);
	~Collectible();

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual shared_ptr<sprite_properties> getSprite() const;
	virtual string itemName() const;

	virtual void onPlayerContact(Player* p);

	local_shared_ptr<MagicEffectDescriptor> getEffect(GObject* target) const;

	bool canAcquire(Player* player);
	void onAcquire(Player* player);
protected:
	string sprite;
	local_shared_ptr<MagicEffectDescriptor> effect;
	effect_attributes attrs;
};

#endif /* Collectibles_hpp */
