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

#undef cons

#endif /* PlayerBullet_hpp */
