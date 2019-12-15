//
//  MapFragment.hpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#ifndef MapFragment_hpp
#define MapFragment_hpp

#include "InventoryObject.hpp"

class MapFragment : public InventoryObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapFragment(GSpace* space, ObjectIDType id, const ValueMap& args);
	virtual inline ~MapFragment() {}

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual inline string getSprite() const { return "mapFragment"; }
	virtual inline string itemName() const { return "Map Fragment"; }

	void onAcquire();

	const int mapFragmentId;
};


#endif /* MapFragment_hpp */
