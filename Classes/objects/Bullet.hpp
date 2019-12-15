//
//  Bullet.hpp
//  Koumachika
//
//  Created by Toni on 11/24/15.
//
//

#ifndef Bullet_hpp
#define Bullet_hpp

#include "Attributes.hpp"
#include "GObject.hpp"

class Agent;
class Wall;

struct bullet_properties
{
	SpaceFloat mass;
	SpaceFloat speed;
	SpaceVect dimensions;
	SpaceFloat knockback;

	DamageInfo damage;

	string sprite;
	string lightSource;

	char hitCount = 1;
	char ricochetCount = 0;
	bool directionalLaunch = true;
	bool ignoreObstacles = false;
	bool deflectBullets = false;

	bullet_properties clone();
};

//The attributes of the creating agent that can affect this object. These are 
//captured from the agent when the bullet object factory is created.
struct bullet_attributes
{
	static bullet_attributes getDefault();

	SpaceVect casterVelocity;
	object_ref<Agent> caster;
	GType type;
	RoomSensor* startRoom = nullptr;

	float attackDamage = 1.0f;
	float bulletSpeed = 1.0f;
};

class Bullet : public GObject
{
public:
	static const bool logRicochets;

	static shared_ptr<object_params> makeParams(
		SpaceVect pos,
		SpaceFloat angle,
		SpaceVect vel = SpaceVect::zero,
		SpaceFloat angularVel  = 0.0
	);

	Bullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes,
		const physics_params& phys
	);
	inline virtual ~Bullet() {}

	virtual inline bool getSensor() const { return true; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

	virtual void initializeGraphics();
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }

	//Interface
	virtual inline DamageInfo getDamageInfo() const { return DamageInfo{}; }
	virtual inline SpaceFloat getKnockbackForce() const { return 0.0; }

	void onWallCollide(Wall* wall);
	void onEnvironmentCollide(GObject* obj);
	void onAgentCollide(Agent* agent, SpaceVect n);
	void onBulletCollide(Bullet* bullet);

	DamageInfo getScaledDamageInfo() const;
	SpaceVect calculateLaunchVelocity();
	bool applyRicochet(SpaceVect n);
	void setBodyVisible(bool b);
	//Shield bullet is no-collide with normal obstacles, and is not consumed
	//upon contactwith an enemy. It may also destroy other bullets
	//(of non-matching type) it collides with.
	void setShield(bool deflectBullets);
protected:
	bullet_attributes attributes;

	int ricochetCount = 0;
	int hitCount = 1;
	bool deflectBullets = false;
	bool ignoreObstacleCollision = false;
};

class BulletImpl : public Bullet
{
public:
	BulletImpl(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes,
		shared_ptr<bullet_properties> props
	);
	inline virtual ~BulletImpl() {}

	virtual void init();

	virtual inline SpaceFloat getKnockbackForce() const { return props->knockback; }
	virtual inline SpaceFloat getMaxSpeed() const { return props->speed; }

	virtual inline string getSprite() const { return props->sprite; }
	virtual inline shared_ptr<LightArea> getLightSource() const { return app::getLight(props->lightSource); }

	virtual inline DamageInfo getDamageInfo() const { return props->damage; }
protected:
	shared_ptr<bullet_properties> props;
};

#endif /* Bullet_hpp */
