//
//  Upgrade.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef Upgrade_hpp
#define Upgrade_hpp

#include "GObject.hpp"

class Upgrade : public GObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	Upgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
	virtual inline ~Upgrade() {}

	virtual inline GType getType() const { return GType::playerPickup; }
	virtual inline bool getSensor() const { return true; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual string getSprite() const;

	//Which particular instance of this upgrade.
	const unsigned int upgrade_id;
	const Attribute attribute;
};

#endif /* Upgrade_hpp */
