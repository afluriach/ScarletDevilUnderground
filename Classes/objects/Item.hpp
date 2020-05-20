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

class item_properties : public object_properties
{
public:
	item_properties();

	string scriptName;
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
	static ObjectGeneratorType create(GSpace* space, string items, SpaceVect pos);

	Item(GSpace* space, ObjectIDType id, const item_attributes& attr, local_shared_ptr<item_properties> props);
	~Item();

	virtual void init();

	inline virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }

	virtual void onPlayerContact(Player* p);
protected:
	local_shared_ptr<item_properties> props;
};

#endif /* Item_hpp */
