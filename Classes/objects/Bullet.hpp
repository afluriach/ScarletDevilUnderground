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
	SpaceFloat speed;
	SpaceVect dimensions;

	DamageInfo damage;

	string sprite;
	string lightSource;

	int hitCount = 1;
	int ricochetCount = 0;

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

	float size = 1.0f;
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
		shared_ptr<bullet_properties> props
	);
	inline virtual ~Bullet() {}

	virtual void init();
	virtual void initializeGraphics();
	virtual inline GraphicsLayer sceneLayer() const { return GraphicsLayer::ground; }
	virtual inline string getSprite() const { return props->sprite; }
	virtual inline shared_ptr<LightArea> getLightSource() const { return app::getLight(props->lightSource); }
	virtual inline SpaceFloat getMaxSpeed() const { return props->speed; }

	inline DamageInfo getDamageInfo() const { return props->damage; }

	void onWallCollide(Wall* wall);
	void onEnvironmentCollide(GObject* obj);
	void onAgentCollide(Agent* agent, SpaceVect n);
	void onBulletCollide(Bullet* bullet);

	DamageInfo getScaledDamageInfo() const;
	SpaceVect calculateLaunchVelocity();
	bool applyRicochet(SpaceVect n);
	void setBodyVisible(bool b);
protected:
	bullet_attributes attributes;
	shared_ptr<bullet_properties> props;

	int ricochetCount = 0;
	int hitCount = 1;
};

#endif /* Bullet_hpp */
