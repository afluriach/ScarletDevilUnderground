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

enum class collectible_id
{
	nil,

	health1,
	health2,

	power1,
	power2,

	magic1,
	magic2,
};

class Collectible : virtual public GObject, public RectangleBody, public ImageSprite
{
public:
	static ObjectGeneratorType create(collectible_id id, SpaceVect pos);
	static const map<collectible_id, function<ObjectGeneratorType(SpaceVect)>> factories;

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

class Magic1 : public CollectibleImpl<Magic1>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Magic1(GSpace* space, ObjectIDType id, SpaceVect pos);
};

class Magic2 : public CollectibleImpl<Magic2>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Magic2(GSpace* space, ObjectIDType id, SpaceVect pos);
};

class Health1 : public CollectibleImpl<Health1>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Health1(GSpace* space, ObjectIDType id, SpaceVect pos);
};

class Health2 : public CollectibleImpl<Health2>
{
public:
	static const AttributeMap effect;
	static const string spriteName;

	Health2(GSpace* space, ObjectIDType id, SpaceVect pos);
};


#endif /* Collectibles_hpp */
