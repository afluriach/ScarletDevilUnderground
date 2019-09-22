//
//  Bomb.hpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#ifndef Bomb_hpp
#define Bomb_hpp

#include "Attributes.hpp"
#include "GObject.hpp"

struct bomb_properties
{
	string sprite;
	string explosionSound;

	float friction = 1.0f / 16.0f;
	float blastRadius;
	float fuseTime;
	DamageInfo damage;
};

class Bomb : public GObject
{
public:
	static const SpaceFloat explosionSpriteRadius;

	Bomb(shared_ptr<object_params> params, shared_ptr<bomb_properties> props);
	inline virtual ~Bomb() {}

	virtual inline SpaceFloat uk() const { return props->friction; }
	virtual inline bool getSensor() const { return false; }
	virtual inline PhysicsLayers getLayers() const { return enum_bitwise_or( PhysicsLayers,ground,floor); }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual inline GType getType() const { return GType::bomb; }

	virtual void init();
	virtual void update();
	void detonate();

	//Interface
	virtual DamageInfo getDamageInfo() const { return props->damage; }
	virtual SpaceFloat getFuseTime() const { return props->fuseTime; }
	virtual SpaceFloat getBlastRadius() const { return props->blastRadius; }

	virtual string getExplosionSound() const { return props->explosionSound; }
	virtual string getSprite() const { return props->sprite; }
protected:
	shared_ptr<bomb_properties> props;

	SpaceFloat countdown;
	ALuint fuseSound = 0;
	bool detonated = false;
};

#endif /* Bomb_hpp */
