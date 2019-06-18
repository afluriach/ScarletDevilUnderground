//
//  Items.hpp
//  Koumachika
//
//  Created by Toni on 5/26/16.
//
//

#ifndef Items_hpp
#define Items_hpp

#include "InventoryObject.hpp"

class ForestBook1 : public InventoryObject
{
public:
	MapObjCons(ForestBook1);
	
	virtual inline string getSprite() const { return "forestBook"; }
	virtual inline string itemName() const { return "ForestBook1"; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
};

class GraveyardBook1 : public InventoryObject
{
public:
	MapObjCons(GraveyardBook1);

	virtual inline string getSprite() const { return "graveyardBook"; }
	virtual inline string itemName() const { return "GraveyardBook1"; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
};

class Spellcard : public InventoryObject, public LightObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapObjCons(Spellcard);

	virtual shared_ptr<LightArea> getLightSource() const;
	virtual void initializeGraphics();
	virtual inline string getSprite() const { return "spellcard"; }
	virtual inline string itemName() const { return name; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
protected:
	string name;
};

#endif /* Items_hpp */
