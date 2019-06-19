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

#define cons(x) x(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent);

class PlayerBullet : virtual public Bullet
{
public:
	inline PlayerBullet() {}
	inline virtual ~PlayerBullet() {}

	virtual inline GType getType() const { return GType::playerBullet; }
};

class PlayerBulletImpl : public PlayerBullet, public BulletImpl
{
public:
	static const bullet_properties flandreBigOrb1;
	static const bullet_properties catadioptricBullet1;
	static const bullet_properties catadioptricBullet2;
	static const bullet_properties catadioptricBullet3;
	static const bullet_properties rumiaFastOrb1;
	static const bullet_properties cirnoSmallIceBullet;
	static const bullet_properties cirnoLargeIceBullet;

	PlayerBulletImpl(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props);
	inline virtual ~PlayerBulletImpl() {}
};

class StarbowBreakBullet : public PlayerBulletImpl, public LightObject
{
public:
	StarbowBreakBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props);

	virtual shared_ptr<LightArea> getLightSource() const;
};

class CatadioptricBullet : public PlayerBulletImpl, public LightObject
{
public:
	CatadioptricBullet(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent, shared_ptr<bullet_properties> props);

	virtual shared_ptr<LightArea> getLightSource() const;
};

class ScarletDagger :
	virtual public GObject,
	public PlayerBullet,
	public RectangleBody,
	public ImageSprite,
	public LightObject,
	public DirectionalLaunch
{
public:
	static const bullet_properties props;

	ScarletDagger(GSpace* space, ObjectIDType id, const SpaceVect& pos, SpaceFloat angle, object_ref<Agent> agent);

	virtual inline SpaceFloat getMass() const { return props.mass; }
	virtual inline SpaceFloat getMaxSpeed() const { return props.speed; }
	virtual inline SpaceFloat getRadius() const { return props.radius; }

	virtual inline DamageInfo getDamageInfo() const { return props.damage; }

	virtual inline string getSprite() const { return "scarletDagger"; }
	virtual shared_ptr<LightArea> getLightSource() const;
};

class FlandreFastOrb1 : public PlayerBullet, public BulletImpl, public LightObject
{
public:
	static const bullet_properties props;

	cons(FlandreFastOrb1);

	virtual shared_ptr<LightArea> getLightSource() const;
};

class FlanPolarBullet :
	public PlayerBullet,
	public BulletImpl,
	public ParametricMotion,
	public LightObject
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
		object_ref<Agent> agent,
		SpaceFloat parametric_start
	);

	virtual void update();
	virtual shared_ptr<LightArea> getLightSource() const;
};

class FlandrePolarMotionOrb :
	public PlayerBullet,
	public BulletImpl,
	public LightObject
{
public:
	static const bullet_properties props;

	cons(FlandrePolarMotionOrb);

	virtual void update();

	virtual shared_ptr<LightArea> getLightSource() const;
};

class Lavaeteinn :
	virtual public GObject,
	public PlayerBullet,
	public ShieldBullet,
	public RectangleBody,
	public ImageSprite
{
public:
	Lavaeteinn(
		GSpace* space,
		ObjectIDType id,
		const SpaceVect& pos,
		SpaceFloat angle,
		SpaceFloat angularVel,
		object_ref<Agent> agent
	);

	virtual inline string getSprite() const { return "lavaeteinn"; }

	virtual DamageInfo getDamageInfo() const;
	inline virtual SpaceFloat getKnockbackForce() const { return 30.0; }

//	virtual void initializeGraphics();
//	virtual void update();
};

class FlandreCounterClockBullet :
	virtual public GObject,
	public PlayerBullet,
	public ShieldBullet,
	public RectangleBody,
	public ImageSprite
{
public:
	cons(FlandreCounterClockBullet);

	virtual inline string getSprite() const { return "flandreCounterClock"; }

	virtual DamageInfo getDamageInfo() const;
	virtual inline SpaceFloat getKnockbackForce() const { return 50.0; }
};

class CirnoIceShieldBullet :
	virtual public GObject,
	public PlayerBullet,
	public ShieldBullet,
	public CircleBody,
	public ImageSprite
{
public:
	static const bullet_properties props;

	cons(CirnoIceShieldBullet);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }
	virtual inline SpaceFloat getRadius() const { return 0.3; }

	virtual inline string getSprite() const { return "cirnoIceBullet"; }

	virtual DamageInfo getDamageInfo() const;

	inline virtual SpaceFloat getKnockbackForce() const { return 99.0; }
};

#undef cons

#endif /* PlayerBullet_hpp */
