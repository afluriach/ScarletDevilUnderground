//
//  Upgrade.hpp
//  Koumachika
//
//  Created by Toni on 1/12/19.
//
//

#ifndef Upgrade_hpp
#define Upgrade_hpp

class Upgrade : public GObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	Upgrade(GSpace* space, ObjectIDType id, const ValueMap& args);
	virtual inline ~Upgrade() {}

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual string getSprite() const;
	virtual string itemName() const;
	void onAcquire(Player* player);

	virtual void onPlayerContact(Player* p);

	//Which particular instance of this upgrade.
	const unsigned int upgrade_id;
	const Attribute attribute;
};

#endif /* Upgrade_hpp */
