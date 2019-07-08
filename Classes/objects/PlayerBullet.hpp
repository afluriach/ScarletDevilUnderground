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

#undef cons

#endif /* PlayerBullet_hpp */
