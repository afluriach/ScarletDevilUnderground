//
//  Collectibles.hpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#ifndef Collectibles_hpp
#define Collectibles_hpp

#include "Attributes.hpp"
#include "GObject.hpp"
#include "GObjectMixins.hpp"

class Collectible : virtual public GObject, public RectangleBody, public ImageSprite
{
public:
	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos);

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::collectible; }
	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceVect getDimensions() const { return SpaceVect(0.5f, 0.5f); }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }

	virtual inline AttributeMap getEffect() const = 0;
};

template<class D>
class CollectibleImpl : public Collectible
{
public:
	inline CollectibleImpl(GSpace* space, ObjectIDType id, SpaceVect pos) :
		GObject(space, id, "", pos, true),
		Collectible(space, id, pos)
	{}

	virtual inline string imageSpritePath() const { return "sprites/" + D::spriteName + ".png"; }
	virtual inline AttributeMap getEffect() const { return D::effect; }
};

class Power1 : public CollectibleImpl<Power1>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Power1(GSpace* space, ObjectIDType id, SpaceVect pos);
};

class Power2 : public CollectibleImpl<Power2>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Power2(GSpace* space, ObjectIDType id, SpaceVect pos);
};


#endif /* Collectibles_hpp */
