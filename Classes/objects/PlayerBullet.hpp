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

#define cons(x) x(shared_ptr<object_params> params, const bullet_attributes& attributes);

class StarbowBreakBullet : public BulletImpl
{
public:
	StarbowBreakBullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes,
		shared_ptr<bullet_properties> props
	);

	virtual shared_ptr<LightArea> getLightSource() const;
};

class CatadioptricBullet : public BulletImpl
{
public:
	CatadioptricBullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes,
		shared_ptr<bullet_properties> props
	);

	virtual shared_ptr<LightArea> getLightSource() const;
};

class ScarletDagger : public Bullet
{
public:
	static const string props;

	ScarletDagger(shared_ptr<object_params> params, const bullet_attributes& attributes);

	virtual inline SpaceFloat getMaxSpeed() const { return app::getBullet("scarletDagger")->speed; }

	virtual inline DamageInfo getDamageInfo() const { return app::getBullet("scarletDagger")->damage; }

	virtual inline string getSprite() const { return "scarletDagger"; }
	virtual shared_ptr<LightArea> getLightSource() const;
};

class FlanPolarBullet : public BulletImpl
{
public:
	static const string props;
	//parametric / angle scale - at 1.0, it takes 2pi seconds
	//to trace the entire graph
	static const SpaceFloat W;
	//magnitude scale
	static const SpaceFloat A;
	//frequency, number of petals
	static const SpaceFloat B;

	static SpaceVect parametric_motion(SpaceFloat t);

	FlanPolarBullet(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes
	);
};

class FlandrePolarMotionOrb : public BulletImpl
{
public:
	static const string props;

	cons(FlandrePolarMotionOrb);

	virtual void update();
};

class Lavaeteinn : public Bullet
{
public:
	Lavaeteinn(
		shared_ptr<object_params> params,
		const bullet_attributes& attributes
	);

	virtual inline string getSprite() const { return "lavaeteinn"; }

	virtual DamageInfo getDamageInfo() const;
	inline virtual SpaceFloat getKnockbackForce() const { return 30.0; }

//	virtual void initializeGraphics();
//	virtual void update();
};

class FlandreCounterClockBullet : public Bullet
{
public:
	cons(FlandreCounterClockBullet);

	virtual inline string getSprite() const { return "flandreCounterClock"; }

	virtual DamageInfo getDamageInfo() const;
	virtual inline SpaceFloat getKnockbackForce() const { return 50.0; }
};

class CirnoIceShieldBullet : public Bullet
{
public:
	static const string props;

	cons(CirnoIceShieldBullet);

	virtual inline SpaceFloat getMaxSpeed() const { return 9.0; }

	virtual inline string getSprite() const { return "cirnoIceBullet"; }

	virtual DamageInfo getDamageInfo() const;

	inline virtual SpaceFloat getKnockbackForce() const { return 99.0; }
};

#undef cons

#endif /* PlayerBullet_hpp */
