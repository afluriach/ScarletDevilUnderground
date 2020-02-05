//
//  Item.hpp
//  Koumachika
//
//  Created by Toni on 2/2/20.
//
//

#ifndef Item_hpp
#define Item_hpp

class LightArea;

class item_properties
{
public:
	item_properties();

	boost::shared_ptr<LightArea> light;
	SpaceVect dimensions;

	string name;
	string scriptName;
	string sprite;
	string onAcquireDialog;

	bool addToInventory = true;
};

struct item_attributes
{
	SpaceVect pos;
	
	string name;
};

class Item : public GObject
{
public:
	static item_attributes parseAttributes(const ValueMap& args);
	static bool conditionalLoad(GSpace* space, const item_attributes& attr, local_shared_ptr<item_properties> props);

	Item(GSpace* space, ObjectIDType id, const item_attributes& attr, local_shared_ptr<item_properties> props);

	inline virtual string getClsName() const { return props->name; }

	virtual void init();

	inline virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	inline virtual string getSprite() const { return props->sprite; }
	inline virtual boost::shared_ptr<LightArea> getLightSource() const { return props->light; }

	virtual void onPlayerContact(Player* p);
protected:
	local_shared_ptr<item_properties> props;
};

#endif /* Item_hpp */
