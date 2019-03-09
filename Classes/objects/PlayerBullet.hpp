//
//  PlayerBullet.hpp
//  Koumachika
//
//  Created by Toni on 1/11/19.
//
//

#ifndef PlayerBullet_hpp
#define PlayerBullet_hpp

#include "Bullet.hpp"

class PlayerBullet : virtual public Bullet
{
public:
	PlayerBullet(Agent* agent);
	inline virtual ~PlayerBullet() {}

	virtual void onAgentCollide(Agent* agent, SpaceVect n);

	virtual inline GType getType() const { return GType::playerBullet; }
};

class PlayerShield : virtual public GObject, public Bullet
{
public:
	PlayerShield(Agent* agent);
	inline virtual ~PlayerShield() {}

	virtual inline GType getType() const { return GType::playerBullet; }

	//Shield bullet is no-collide with normal obstacles, and is not consumed
	//upon contactwith an enemy.
	virtual void onWallCollide(Wall* wall);
	virtual void onEnvironmentCollide(GObject* obj);
	virtual void onAgentCollide(Agent* agent, SpaceVect n);
	virtual void onBulletCollide(Bullet* bullet);

	virtual SpaceFloat getKnockbackForce() const = 0;
};

class FlandreBigOrb1 : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	FlandreBigOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);
};

class FlandreFastOrb1 : public PlayerBullet, public BulletImpl, public RadialLightObject
{
public:
	static const bullet_properties props;

	FlandreFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 }; }
};

class FlandrePolarMotionOrb :
	virtual public GObject,
	public PlayerBullet,
	public CircleBody,
	public ImageSprite,
	public DirectionalLaunch,
	public RadialLightObject,
	public RegisterUpdate<FlandrePolarMotionOrb>
{
public:
	FlandrePolarMotionOrb(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	void update();

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 }; }

	virtual inline SpaceFloat getMass() const { return 1.0; }
	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.15; }

	virtual inline string imageSpritePath() const { return "sprites/flandre_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;
};

class FlandreCounterClockBullet :
	virtual public GObject,
	public PlayerShield,
	public RectangleBody,
	public ImageSprite
{
public:
	FlandreCounterClockBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline string imageSpritePath() const { return "sprites/counter_clock_bullet.png"; }
	virtual inline float zoom() const { return 0.25f; }

	virtual AttributeMap getAttributeEffect() const;
	virtual inline SpaceFloat getKnockbackForce() const { return 0.0; }
};

class RumiaFastOrb1 : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	RumiaFastOrb1(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);
};

class CirnoSmallIceBullet : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	CirnoSmallIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);
};

class CirnoLargeIceBullet : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties props;

	CirnoLargeIceBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);
};

class CirnoIceShieldBullet :
	virtual public GObject,
	public PlayerShield,
	public CircleBody,
	public ImageSprite
{
public:
	static const bullet_properties props;

	CirnoIceShieldBullet(GSpace* space, ObjectIDType id, Agent* agent, SpaceFloat angle, const SpaceVect& pos);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;

	inline virtual SpaceFloat getKnockbackForce() const { return 99.0; }
};

#endif /* PlayerBullet_hpp */
