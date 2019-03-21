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

class Upgrade : virtual public GObject, public CircleBody, public ImageSprite
{
public:
	Upgrade(GSpace* space, ObjectIDType id, const ValueMap& args, Attribute at);
	virtual inline ~Upgrade() {}

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::playerPickup; }
	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getRadius() const { return 0.75; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual string imageSpritePath() const;

	void init();

	//Which particular instance of this upgrade.
	const unsigned int upgrade_id;
	const Attribute attribute;
};

#endif /* Upgrade_hpp */
