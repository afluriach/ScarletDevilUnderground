//
//  Item.hpp
//  Koumachika
//
//  Created by Toni on 2/2/20.
//
//

#ifndef Item_hpp
#define Item_hpp

class item_properties : public object_properties
{
public:
	item_properties();
    virtual ~item_properties();

	string scriptName;
	string onAcquireDialog;

	bool addToInventory = true;

	inline virtual type_index getType() const { return typeid(*this); }
};

class Item : public GObject
{
public:
	static bool conditionalLoad(GSpace* space, const object_params& params, local_shared_ptr<item_properties> props);
	static ObjectGeneratorType create(GSpace* space, string items, SpaceVect pos);

	Item(GSpace* space, ObjectIDType id, const object_params& params, local_shared_ptr<item_properties> props);
	~Item();

	virtual void init();

	inline virtual GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }

	virtual void onPlayerContact(Player* p);
protected:
	local_shared_ptr<item_properties> props;
};

#endif /* Item_hpp */
