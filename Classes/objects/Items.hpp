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
	
	virtual inline string imageSpritePath() const { return "sprites/items/forest_book.png"; }
	virtual inline string itemName() const { return "ForestBook1"; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
};

class GraveyardBook1 : public InventoryObject
{
public:
	MapObjCons(GraveyardBook1);

	virtual inline string imageSpritePath() const { return "sprites/items/graveyard_book.png"; }
	virtual inline string itemName() const { return "GraveyardBook1"; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
};

class Spellcard : public InventoryObject
{
public:
	MapObjCons(Spellcard);

	virtual void initializeGraphics();
	virtual inline string imageSpritePath() const { return "sprites/magic_card_empty.png"; }
	virtual inline string itemName() const { return name; }

	inline virtual bool canAcquire() { return true; }
	virtual void onAcquire();
protected:
	string name;
};

#endif /* Items_hpp */
