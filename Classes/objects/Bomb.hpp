//
//  Bomb.hpp
//  Koumachika
//
//  Created by Toni on 2/12/19.
//
//

#ifndef Bomb_hpp
#define Bomb_hpp

struct bomb_properties
{
	string sprite;
	string explosionSound;

	float friction = 1.0f / 16.0f;
	float blastRadius;
	float fuseTime;
	float cost;

	DamageInfo damage;
};

class Bomb : public GObject
{
public:
	static const SpaceFloat explosionSpriteRadius;

	Bomb(local_shared_ptr<object_params> params, local_shared_ptr<bomb_properties> props);
	inline virtual ~Bomb() {}

	virtual inline SpaceFloat uk() const { return props->friction; }
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	virtual void init();
	virtual void update();
	void detonate();

	//Interface
	inline DamageInfo getDamageInfo() const { return props->damage; }
	inline SpaceFloat getFuseTime() const { return props->fuseTime; }
	inline SpaceFloat getBlastRadius() const { return props->blastRadius; }

	inline string getExplosionSound() const { return props->explosionSound; }

	inline virtual string getSprite() const { return props->sprite; }
	virtual bool hit(DamageInfo damage, SpaceVect n);
protected:
	local_shared_ptr<bomb_properties> props;

	SpaceFloat countdown;
	ALuint fuseSound = 0;
	bool detonated = false;
};

#endif /* Bomb_hpp */
