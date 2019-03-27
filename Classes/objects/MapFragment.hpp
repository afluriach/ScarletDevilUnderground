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
#include "GObjectMixins.hpp"

class MapFragment : virtual public GObject, public CircleBody, public ImageSprite
{
public:
	static bool conditionalLoad(GSpace* space, ObjectIDType id, const ValueMap& args);

	MapFragment(GSpace* space, ObjectIDType id, const ValueMap& args);
	virtual inline ~MapFragment() {}

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::playerPickup; }
	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getRadius() const { return 0.5; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual inline string imageSpritePath() const { return "sprites/map.png"; }

	void onAcquire();

	const int mapFragmentId;
};


#endif /* MapFragment_hpp */
