//
//  FirePatternImpl.hpp
//  Koumachika
//
//  Created by Toni on 7/10/19.
//
//

#ifndef FirePatternImpl_hpp
#define FirePatternImpl_hpp

#include "FirePattern.hpp"

struct bullet_emitter
{
	SpaceVect offset;
	SpaceFloat angle;
};

struct firepattern_properties
{
	static shared_ptr<firepattern_properties> makeSingle(
		shared_ptr<bullet_properties> props,
		SpaceFloat interval,
		SpaceFloat distance,
		SpaceFloat burstInterval = 0.0,
		int burstCount = 1
	);
	static shared_ptr<firepattern_properties> makeRadius(
		shared_ptr<bullet_properties> props,
		SpaceFloat interval,
		SpaceFloat distance,
		int count
	);
	static shared_ptr<firepattern_properties> makeSpread(
		shared_ptr<bullet_properties> props,
		SpaceFloat interval,
		SpaceFloat distance,
		SpaceFloat burstInterval,
		int burstCount,
		SpaceFloat sideAngle,
		int emitterCount
	);

	vector<bullet_emitter> emitters;
	shared_ptr<bullet_properties> bullet;

	SpaceFloat fireInterval;
	SpaceFloat burstInterval;
	int burstCount = 1;

	string icon;
	float staminaCost = 0.0f;

	bool rotate = true;
};

class FirePatternImpl : public FirePattern
{
public:
	FirePatternImpl(Agent *const agent, shared_ptr<firepattern_properties> props);
	virtual inline ~FirePatternImpl() {}

	inline virtual string iconPath() const { return props->icon; }
	inline virtual float getCost() const { return props->staminaCost; }

	virtual bool fire();
	virtual void update();
protected:
	bool createBullet(SpaceVect offset, SpaceFloat angle);
	bool emit(SpaceVect origin, SpaceFloat angle, const bullet_emitter& be);

	shared_ptr<firepattern_properties> props;

	float cooldownTimer = 0.0f;
	int crntBurstCount = 0;
};

#endif
