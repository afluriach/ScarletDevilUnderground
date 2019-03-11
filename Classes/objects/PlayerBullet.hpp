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

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent);

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

class PlayerBulletImpl : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties flandreBigOrb1;
	static const bullet_properties rumiaFastOrb1;
	static const bullet_properties cirnoSmallIceBullet;
	static const bullet_properties cirnoLargeIceBullet;

	PlayerBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, Agent* agent, const bullet_properties* props);
};

class FlandreFastOrb1 : public PlayerBullet, public BulletImpl, public RadialLightObject
{
public:
	static const bullet_properties props;

	cons(FlandreFastOrb1);

	virtual inline CircleLightArea getLightSource() const { return CircleLightArea{ getPos(), 2.0, Color4F::RED*0.5f, 0.0 }; }
};

class FlanPolarBullet :
	public PlayerBullet,
	public BulletImpl,
	public ParametricMotion,
	public RadialLightObject
{
public:
	static const bullet_properties props;
	//parametric / angle scale - at 1.0, it takes 2pi seconds
	//to trace the entire graph
	static const SpaceFloat W;
	//magnitude scale
	static const SpaceFloat A;
	//frequency, number of petals
	static const SpaceFloat B;

	static SpaceVect parametric_motion(SpaceFloat t);

	cons(FlanPolarBullet);
	FlanPolarBullet(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		Agent* agent,
		SpaceFloat parametric_start
	);

	virtual void update();
	virtual CircleLightArea getLightSource() const;
	virtual inline void onWallCollide(Wall* wall) {}
	virtual inline void onEnvironmentCollide(GObject* obj) {}
};

class FlandrePolarMotionOrb :
	public PlayerBullet,
	public BulletImpl,
	public RadialLightObject
{
public:
	static const bullet_properties props;

	cons(FlandrePolarMotionOrb);

	virtual void update();

	virtual CircleLightArea getLightSource() const;
};

class FlandreCounterClockBullet :
	virtual public GObject,
	public PlayerShield,
	public RectangleBody,
	public ImageSprite
{
public:
	cons(FlandreCounterClockBullet);

	virtual inline string imageSpritePath() const { return "sprites/counter_clock_bullet.png"; }
	virtual inline float zoom() const { return 0.25f; }

	virtual AttributeMap getAttributeEffect() const;
	virtual inline SpaceFloat getKnockbackForce() const { return 0.0; }
};

class CirnoIceShieldBullet :
	virtual public GObject,
	public PlayerShield,
	public CircleBody,
	public ImageSprite
{
public:
	static const bullet_properties props;

	cons(CirnoIceShieldBullet);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string imageSpritePath() const { return "sprites/cirno_large_ice_bullet.png"; }

	static constexpr float spriteBaseRadius = 0.83f;
	inline virtual float zoom() const { return getRadius() / spriteBaseRadius * 2; }

	virtual AttributeMap getAttributeEffect() const;

	inline virtual SpaceFloat getKnockbackForce() const { return 99.0; }
};

#undef cons

#endif /* PlayerBullet_hpp */
