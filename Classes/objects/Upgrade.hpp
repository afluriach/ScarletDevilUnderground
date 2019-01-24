//
//  Upgrade.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef Upgrade_hpp
#define Upgrade_hpp

#include "Attributes.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"
#include "value_map.hpp"

class Upgrade : virtual public GObject, public CircleBody, public ImageSprite, public RegisterInit<Upgrade>
{
public:
	Upgrade(unsigned int id, Attribute attribute);
	virtual inline ~Upgrade() {}

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::upgrade; }
	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getRadius() const { return 0.75; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }

	void init();

	//Which particular instance of this upgrade.
	const unsigned int upgrade_id;
	const Attribute attribute;
};

template<class D>
class UpgradeImpl : public Upgrade
{
public:
	inline UpgradeImpl(GSpace* space, ObjectIDType id, const ValueMap& args) :
		MapObjForwarding(GObject),
		Upgrade(getInt(args, "id"), D::attribute)
	{}

	virtual inline string imageSpritePath() const { return "sprites/" + D::spriteName + ".png"; }
};

class HPUpgrade : public UpgradeImpl<HPUpgrade>
{
public:
	static const Attribute attribute;
	static const string spriteName;

	HPUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
};

class MPUpgrade : public UpgradeImpl<MPUpgrade>
{
public:
	static const Attribute attribute;
	static const string spriteName;

	MPUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
};

class PowerUpgrade : public UpgradeImpl<PowerUpgrade>
{
public:
	static const Attribute attribute;
	static const string spriteName;

	PowerUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
};

class AgilityUpgrade : public UpgradeImpl<AgilityUpgrade>
{
public:
	static const Attribute attribute;
	static const string spriteName;

	AgilityUpgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
};

#endif /* Upgrade_hpp */
