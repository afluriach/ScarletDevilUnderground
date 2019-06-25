//
//  MapFragment.hpp
//  Koumachika
//
//  Created by Toni on 3/27/19.
//
//

#ifndef MapFragment_hpp
#define MapFragment_hpp

#include "GObject.hpp"

class MapFragment : public GObject
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapFragment(GSpace* space, ObjectIDType id, const ValueMap& args);
	virtual inline ~MapFragment() {}

	virtual inline GType getType() const { return GType::playerPickup; }
	virtual inline bool getSensor() const { return true; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual inline string getSprite() const { return "mapFragment"; }

	void onAcquire();

	const int mapFragmentId;
};


#endif /* MapFragment_hpp */
