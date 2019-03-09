//
//  FirePattern.hpp
//  Koumachika
//
//  Created by Toni on 11/22/18.
//
//

#ifndef FirePattern_hpp
#define FirePattern_hpp

#include "Agent.hpp"

class FirePattern
{
public:
	inline FirePattern(Agent *const agent) : agent(agent) {}
	virtual inline ~FirePattern() {}

	virtual bool fireIfPossible();
	virtual bool isInCooldown();
	virtual void update();
	virtual SpaceFloat getLaunchDistance() const { return 1.0; }

	virtual bool fire() = 0;
	virtual bool spawn(SpaceFloat angle) = 0;
	virtual bool spawn(SpaceVect posOffset, SpaceFloat angle) = 0;
	virtual float getCooldownTime() = 0;
	virtual string iconPath() const = 0;
protected:
	float cooldownTimeRemaining = 0;
	Agent * const agent;
};

template<class C>
class FirePatternImpl : virtual public FirePattern
{
public:
	inline FirePatternImpl() {}
	virtual inline ~FirePatternImpl() {}

	inline virtual bool spawn(SpaceFloat angle)
	{
		return agent->bulletCheckSpawn<C>(
			agent->getPos() + SpaceVect::ray(getLaunchDistance(), angle),
			angle
		).isValid();
	}

	inline virtual bool spawn(SpaceVect posOffset, SpaceFloat angle)
	{
		return agent->bulletCheckSpawn<C>(
			agent->getPos() + SpaceVect::ray(getLaunchDistance(), angle) + posOffset,
			angle
		).isValid();
	}

};

class SingleBulletFixedIntervalPattern : virtual public FirePattern
{
public:
	inline SingleBulletFixedIntervalPattern(Agent *const agent) : FirePattern(agent) {}
	virtual inline ~SingleBulletFixedIntervalPattern() {}

	virtual bool fire();
};

class RadiusPattern : virtual public FirePattern
{
public:
	RadiusPattern(
		Agent *const agent,
		float fireInterval,
		int bulletCount
	);
	virtual inline ~RadiusPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	int bulletCount;
	float fireInterval;
};

class MultiBulletSpreadPattern : virtual public FirePattern
{
public:
	MultiBulletSpreadPattern(
		Agent *const agent,
		float fireInterval, 
		SpaceFloat sideAngleSpread,
		int bulletCount
	);
	virtual inline ~MultiBulletSpreadPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	SpaceFloat sideAngleSpread;
	int bulletCount;
	float fireInterval;
};

class MultiBulletParallelPattern : virtual public FirePattern
{
public:
	MultiBulletParallelPattern(
		Agent *const agent,
		float fireInterval,
		SpaceFloat bulletSpacing,
		int bulletCount
	);
	virtual inline ~MultiBulletParallelPattern() {}

	virtual bool fire();
	inline virtual float getCooldownTime() { return fireInterval; }
protected:
	SpaceFloat bulletSpacing;
	int bulletCount;
	float fireInterval;
};

class BurstPattern : virtual public FirePattern
{
public:
	BurstPattern(
		Agent *const agent,
		float burstInterval,
		float burstLength,
		int bulletsPerBurst
	);
	virtual inline ~BurstPattern() {}

	virtual bool isInCooldown();
	virtual void update();
	virtual bool fire();

	inline virtual float getCooldownTime() { return 0.0f; }
protected:
	int crntBurstCount = 0 ;
	float countdownTimer = 0.0f;

	const float burstInterval;
	const float burstLength;
	const int bulletsPerBurst;
};


#endif /* FirePattern_hpp */
