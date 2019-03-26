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
#include "types.h"

class Agent;
class Wall;

struct bullet_properties
{
	SpaceFloat mass;
	SpaceFloat speed;
	SpaceFloat radius;

	AttributeMap attributeEffect;

	SpaceFloat spriteBaseRadius;
	string sprite;
	Color3B spriteColor;

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
	virtual inline AttributeMap getAttributeEffect() const { return AttributeMap(); }
	virtual inline shared_ptr<MagicEffect> getMagicEffect(gobject_ref target) { return nullptr; }

	virtual void onWallCollide(Wall* wall);
	virtual void onEnvironmentCollide(GObject* obj);
	virtual void onAgentCollide(Agent* agent, SpaceVect n);
	virtual void onBulletCollide(Bullet* bullet);

	SpaceVect calculateLaunchVelocity();
	bool applyRicochet(SpaceVect n);
	void setBodyVisible(bool b);

	object_ref<Agent> agent;
	//modifier attributes, should be copied at time bullet is created.
	bullet_attributes attributes;

	SpaceFloat knockback = 0.0;
	int ricochetCount = 0;
	int hitCount = 1;
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
	virtual inline string imageSpritePath() const { return props->sprite; }
	virtual inline float zoom() const { return props->radius / props->spriteBaseRadius * 2.0f; }

	virtual inline AttributeMap getAttributeEffect() const { return props->attributeEffect; }

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
	virtual inline string imageSpritePath() const { return props.sprite; }
	virtual inline float zoom() const { return props.radius / props.spriteBaseRadius * 2.0f; }

	virtual inline AttributeMap getAttributeEffect() const { return props.attributeEffect; }

	const bullet_properties props;
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
