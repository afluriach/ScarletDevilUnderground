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
	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, const string& spriteName, AttributeMap effect);

	virtual inline SpaceFloat getMass() const { return -1.0; }
	virtual inline GType getType() const { return GType::collectible; }
	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceVect getDimensions() const { return SpaceVect(0.5f, 0.5f); }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual inline string imageSpritePath() const { return "sprites/"+spriteName+".png"; }

	virtual inline AttributeMap getEffect() const { return effect; }
protected:
	string spriteName;
	AttributeMap effect;
};

class Power1 : public Collectible
{
public:
	static const AttributeMap effect;

	Power1(GSpace* space, ObjectIDType id, SpaceVect pos);
};

class Power2 : public Collectible
{
public:
	static const AttributeMap effect;

	Power2(GSpace* space, ObjectIDType id, SpaceVect pos);
};


#endif /* Collectibles_hpp */
