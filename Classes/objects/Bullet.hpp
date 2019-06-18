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
#include "GObjectMixins.hpp"

class Agent;
class Wall;

struct bullet_properties
{
	SpaceFloat mass;
	SpaceFloat speed;
	SpaceFloat radius;

	DamageInfo damage;

	string sprite;

	char hitCount = 1;
	char ricochetCount = 0;
	bool directionalLaunch = true;
};

//The attributes of the creating agent that can affect this object. These are 
//captured from the agent when the bullet object factory is created.
struct bullet_attributes
{
	float attackDamage = 1.0f;
	float bulletSpeed = 1.0f;
};

class Bullet : virtual public GObject
{
public:
	static const bool logRicochets;

	Bullet(object_ref<Agent> agent);
	inline virtual ~Bullet() {}

	virtual inline bool getSensor() const { return true; }
	virtual inline SpaceFloat getMass() const { return 0.1; }
	virtual inline PhysicsLayers getLayers() const { return PhysicsLayers::ground; }

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

	object_ref<Agent> agent;
	//modifier attributes, should be copied at time bullet is created.
	bullet_attributes attributes;

	int ricochetCount = 0;
	int hitCount = 1;
	bool deflectBullets = false;
	bool ignoreObstacleCollision = false;
};

class BulletImpl :
	virtual public Bullet,
	public CircleBody,
	public ImageSprite,
	public RegisterInit<BulletImpl>
{
public:
	BulletImpl(const bullet_properties* props);
	inline virtual ~BulletImpl() {}

	void init();

	virtual inline SpaceFloat getMass() const { return props->mass; }
	virtual inline SpaceFloat getMaxSpeed() const { return props->speed; }
	virtual inline SpaceFloat getRadius() const { return props->radius; }

	virtual void initializeGraphics();
	virtual inline string getSprite() const { return props->sprite; }

	virtual inline DamageInfo getDamageInfo() const { return props->damage; }

	const bullet_properties* props;
};

class BulletValueImpl :
	virtual public Bullet,
	public CircleBody,
	public ImageSprite,
	public RegisterInit<BulletValueImpl>
{
public:
	BulletValueImpl(bullet_properties props);
	inline virtual ~BulletValueImpl() {}

	void init();

	virtual inline SpaceFloat getMass() const { return props.mass; }
	virtual inline SpaceFloat getMaxSpeed() const { return props.speed; }
	virtual inline SpaceFloat getRadius() const { return props.radius; }

	virtual void initializeGraphics();
	virtual inline string getSprite() const { return props.sprite; }

	virtual inline DamageInfo getDamageInfo() const { return props.damage; }

	const bullet_properties props;
};

//Shield bullet is no-collide with normal obstacles, and is not consumed
//upon contactwith an enemy. It may also destroy other bullets
//(of non-matching type) it collides with.
class ShieldBullet : virtual public Bullet
{
public:
	ShieldBullet(object_ref<Agent> agent, bool deflectBullets);
	inline virtual ~ShieldBullet() {}
};


//Object will automatically have its velocity set on init(), according to
//its [facing] angle. Uses polymorphic getter getMaxSpeed().
class DirectionalLaunch : virtual public Bullet, public RegisterInit<DirectionalLaunch>
{
public:
	inline DirectionalLaunch() : RegisterInit<DirectionalLaunch>(this) {}
	void init();
};

#endif /* Bullet_hpp */
