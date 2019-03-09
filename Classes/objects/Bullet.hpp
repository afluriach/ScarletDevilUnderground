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
	SpaceFloat spriteBaseRadius;

	string sprite;
	AttributeMap attributeEffect;

	bool directionalLaunch = true;
};

class Bullet : virtual public GObject
{
public:
	static const bool logRicochets;

	Bullet(Agent* agent);
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

	bool applyRicochet(SpaceVect n);

	void setBodyVisible(bool b);

	SpaceFloat knockback = 0.0;
	float agentAttackMultiplier = 1.0f;
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

	void init();

	virtual inline SpaceFloat getMass() const { return props->mass; }
	virtual inline SpaceFloat getMaxSpeed() const { return props->speed; }
	virtual inline SpaceFloat getRadius() const { return props->radius; }

	virtual inline string imageSpritePath() const { return props->sprite; }
	virtual inline float zoom() const { return props->radius / props->spriteBaseRadius * 2.0f; }

	virtual inline AttributeMap getAttributeEffect() const { return props->attributeEffect; }

	const bullet_properties* props;
};

#endif /* Bullet_hpp */
