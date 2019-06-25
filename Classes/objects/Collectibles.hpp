//
//  Collectibles.hpp
//  Koumachika
//
//  Created by Toni on 4/14/18.
//
//

#ifndef Collectibles_hpp
#define Collectibles_hpp

#include "GObject.hpp"

enum class collectible_id
{
	nil,

	health1,
	health2,
	health3,

	magic1,
	magic2,
	magic3,

	//random health or magic
	hm1,
	hm2,
	hm3,

	key,
};

struct collectible_properties
{
	string sprite;

	Attribute attr;
	float val;
};

class Collectible : public GObject
{
public:
	static const boost::bimap<collectible_id, string> collectibleNameMap;

	static const unordered_map<collectible_id, collectible_properties> propertiesMap;
	static const unordered_map<collectible_id, function<collectible_id(GSpace*)>> vMap;

	static ObjectGeneratorType create(GSpace* space, collectible_id id, SpaceVect pos);
	static collectible_id getCollectibleID(const string& name);

	Collectible(GSpace* space, ObjectIDType id, SpaceVect pos, collectible_id collectibleID);
	inline virtual ~Collectible() {}

	virtual inline GType getType() const { return GType::playerPickup; }
	virtual inline bool getSensor() const { return true; }

	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::floor; }
	virtual string getSprite() const;

	AttributeMap getEffect() const;
protected:
	collectible_id collectibleID;
};

#endif /* Collectibles_hpp */
